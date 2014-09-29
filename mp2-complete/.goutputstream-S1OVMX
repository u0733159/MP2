#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
FramePool * PageTable::kernel_mem_pool = NULL;
FramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(FramePool * _kernel_mem_pool,
                            FramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;

   process_mem_pool = _process_mem_pool;

   assert(_shared_size % PAGE_SIZE == 0);
   shared_size = _shared_size;

   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   unsigned long int * page_table;

   page_directory = (unsigned long int *)(kernel_mem_pool->get_frame() *
                                           PAGE_SIZE);

   page_table = (unsigned long int *)(kernel_mem_pool->get_frame() *
                                      PAGE_SIZE);

   unsigned long sharedPages = shared_size / PAGE_SIZE;      //PAGE_SIZE = 4096 bytes

   int i;
   unsigned long int page_address = 0;

   page_directory[0] = (unsigned long)page_table;
   page_directory[0] |= 0x3;       //011 mode
   //print_page_directory_entry(0);

   for (i=0; i < sharedPages; i++) {
      page_table[i] = page_address;
      page_table[i] |= 0x3;
      page_address += PAGE_SIZE;
   }
   //print_page_table_entry(0,0);


   for (i=1; i<ENTRIES_PER_PAGE; i++) {
      page_directory[i] = 0x2;
   }
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long)page_directory);

   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   write_cr0(read_cr0() | 0x80000000);
   Console::puts("Enabled paging\n");
}

#define PFPRINT
void PageTable::handle_fault(REGS * _r)
{
#ifdef PFPRINT
   Console::puts("Page Fault Handler\n");
#endif
   unsigned long addr = read_cr2();
   unsigned long pageNum = addr >> 12;
   unsigned long free_frame;
   unsigned long free_frame_addr;

#ifdef PFPRINT
   Console::puts("Faulting page number: ");
   Console::putui(pageNum);
   Console::puts("\n");

   Console::puts("Faulting address: ");
   Console::putui(addr);
   Console::puts("\n");
#endif
   /*
    * Does the page table page exist?
    */
   unsigned int page_dir_indx = addr >> 22;
   unsigned int page_table_indx = (addr >> 12) & (0x000003FF);
   unsigned long int *page_table_page_addr = (unsigned long int *)(current_page_table->page_directory[page_dir_indx] &
                                        0xFFFFF000);

   if((current_page_table->page_directory[page_dir_indx] & 0x1) != 0x1) {
      page_table_page_addr = (unsigned long int *)(kernel_mem_pool->get_frame() * PAGE_SIZE);
      current_page_table->page_directory[page_dir_indx] = (unsigned long int)page_table_page_addr;
      current_page_table->page_directory[page_dir_indx] |= 0x3;
#ifdef PFPRINT
      Console::puts("New page directory entry: ");
      Console::putui(current_page_table->page_directory[page_dir_indx]);
      Console::puts("\n");
#endif
   }
   free_frame = process_mem_pool->get_frame();
   free_frame_addr = free_frame * PAGE_SIZE;
#ifdef PFPRINT
   Console::puts("Using frame:");
   Console::putui(free_frame);
   Console::puts("\n");
#endif
   page_table_page_addr[page_table_indx] = free_frame_addr | 0x3;
}

#ifdef _UNDEF_
void PageTable::print_page_directory_entry(int index)
{
   int i=0;
   Console::puts("--PAGE-DIRECTORY(");
   Console::putui(index);
   Console::puts(")--\n");
   Console::putui(page_directory[index]);
   Console::puts("\n");
}


void PageTable::print_page_table_entry(int index, int entry)
{
   unsigned int i;
   unsigned long int *page_table = (unsigned long int *)(page_directory[index] & 0xFFFFF000);

   Console::puts("--PAGE-TABLE(");
   Console::putui(index);
   Console::puts(", ");
   Console::putui(entry);
   Console::puts(")--\n");
   Console::putui(page_table[entry]);
   Console::puts("\n");
}
#endif
