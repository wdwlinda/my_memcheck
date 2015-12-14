#include "level3.hh"

bool Mapped::compare_address (Mapped first, Mapped second)
{
        char* first_addr = (char*)first.mapped_begin();
        char* second_addr = (char*)second.mapped_begin();
        return first_addr < second_addr;
}


bool Mapped::area_contains(void* addr) const
{
        return (char*)addr < (char*)mapped_begin_ + mapped_length_
                             && (char*)addr >= (char*)mapped_begin_;
}


bool Tracker::of_interest(int syscall) const
{
        return syscall == MMAP_SYSCALL || syscall == MREMAP_SYSCALL
                || syscall == MUNMAP_SYSCALL || syscall == MPROTECT_SYSCALL
                || syscall == BRK_SYSCALL;
}

int Tracker::handle_mmap(int syscall, Breaker& b, void* bp)
{
        print_syscall(pid, syscall);
        int retval = b.handle_bp(bp, false);
        if (retval > NO_SYSCALL)
                print_retval(pid);
        return retval;
}

int Tracker::handle_syscall(int syscall, Breaker& b, void* bp)
{
        switch (syscall)
        {
                case MMAP_SYSCALL:
                        return handle_mmap(syscall, b, bp);
                default:
                        return b.handle_bp(bp, false);

        }

        return NO_SYSCALL;
}

void Tracker::print_mapped_areas() const
{
        int i = 0;
        for (auto it = mapped_areas.begin(); it != mapped_areas.end(); it++)
        {
                fprintf(OUT, "Mapped area #%d\n", i);
                fprintf(OUT, "\tBegins:\t%p\n", it->mapped_begin());
                fprintf(OUT, "\tLength:\t%ld\n", it->mapped_length());
                fprintf(OUT, "\tEnds  :\t%p\n", (char*)it->mapped_begin()
                        + it->mapped_length());
                fprintf(OUT, "\tProtections:\t%d\n", it->mapped_protections());
        }

}
