#include "dig_into_mem.hh"

void* get_pt_dynamic(unsigned long phent, unsigned long phnum,
                     pid_t pid_child, void* at_phdr)
{
  // Loop on the Program header until the PT_DYNAMIC entry
  Elf64_Dyn* dt_struct = NULL;
  struct iovec local;
  struct iovec remote;
  char buffer[128];
  Elf64_Phdr* phdr;
  for (unsigned i = 0; i < phnum; ++i)
  {
    local.iov_base = buffer;
    local.iov_len  = sizeof (Elf64_Phdr);
    remote.iov_base = (char*)at_phdr + i * phent;
    remote.iov_len  = sizeof (Elf64_Phdr);

    process_vm_readv(pid_child, &local, 1, &remote, 1, 0);

    phdr = reinterpret_cast<Elf64_Phdr*>(buffer);
    if (phdr->p_type == PT_DYNAMIC)
    {
      // First DT_XXXX entry
      dt_struct = reinterpret_cast<Elf64_Dyn*>(phdr->p_vaddr);
      break;
    }
  }

  if (!dt_struct)
    throw std::logic_error("PT_DYNAMIC not found");

  printf("Found _DYNAMIC:\t\t%p\n", (void*)dt_struct);
  return (void*) dt_struct;
}


void* get_phdr(unsigned long& phent, unsigned long& phnum, pid_t pid_child)
{
    // Open proc/[pid]/auxv
  std::ostringstream ss;
  fprintf(OUT, "Pid %d\n", getpid());
  ss << "/proc/" << pid_child << "/auxv";
  auto file = ss.str();
  int fd = open(file.c_str(), std::ios::binary);
  ElfW(auxv_t) auxv_;

  void* at_phdr;

  // Read from flux until getting all the interesting data
  while (read(fd, &auxv_, sizeof (auxv_)) > -1)
  {
    if (auxv_.a_type == AT_PHDR)
      at_phdr = (void*)auxv_.a_un.a_val;

    if (auxv_.a_type == AT_PHENT)
      phent = auxv_.a_un.a_val;

    if (auxv_.a_type == AT_PHNUM)
      phnum = auxv_.a_un.a_val;

    if (phnum && phent && at_phdr)
      break;
  }
  close(fd);

  return at_phdr;
}

void* get_link_map(void* rr_debug, pid_t pid, int* status)
{
  char buffer[128];
  struct iovec local;
  struct iovec remote;
  local.iov_base  = buffer;
  local.iov_len   = sizeof (struct r_debug);
  remote.iov_base = rr_debug;
  remote.iov_len  = sizeof (Elf64_Phdr);

  process_vm_readv(pid, &local, 1, &remote, 1, 0);

  struct link_map *link_map = reinterpret_cast<struct r_debug*>(buffer)->r_map;

  fprintf(OUT, "Found r_debug->r_map:\t\t%p\n", (void*)link_map);
  *status = reinterpret_cast<struct r_debug*>(buffer)->r_state;
  return link_map;
}

void print_string_from_mem(void* str, pid_t pid)
{
  char s[64] = {0};
  struct iovec local;
  struct iovec remote;
  local.iov_base  = &s;
  local.iov_len   = sizeof (struct link_map);
  remote.iov_base = str;
  remote.iov_len  = sizeof (struct link_map);

  ssize_t read = process_vm_readv(pid, &local, 1, &remote, 1, 0);

  if (read)
    fprintf(OUT, "%s\n", s);

}

void browse_link_map(void* link_m, pid_t pid)
{
  struct link_map map;
  struct iovec local;
  struct iovec remote;
  local.iov_base  = &map;
  local.iov_len   = sizeof (struct link_map);
  remote.iov_base = link_m;
  remote.iov_len  = sizeof (struct link_map);

  process_vm_readv(pid, &local, 1, &remote, 1, 0);

  fprintf(OUT, "\n%sBrowsing link map%s:\n", YELLOW, NONE);

  do
  {
    process_vm_readv(pid, &local, 1, &remote, 1, 0);

    fprintf(OUT, "--> ");
    print_string_from_mem(map.l_name, pid);
    remote.iov_base = map.l_next;
  } while (map.l_next);

  fprintf(OUT, "\n");

}