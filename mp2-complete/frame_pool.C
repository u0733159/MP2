#include "frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "machine.H"

//nsigned long FramePool :: bitmap[8192] {};
unsigned long FramePool::base_frame_no;
unsigned long FramePool::nframes;
unsigned long FramePool::nFreeFrames;
unsigned long FramePool::info_frame_no;
unsigned char* FramePool::bitmap;
FramePool::FramePool(unsigned long _base_frame_no,
          unsigned long _nframes,
          unsigned long _info_frame_no)
{
   unsigned int i;
   base_frame_no = _base_frame_no;
   nframes = _nframes;
   info_frame_no = _info_frame_no;
   /* Decide if the bitmap will fit in a frame.
    */
   if (_nframes > Machine::PAGE_SIZE * 8) {
      Console::puts("Error, Need more than one frame for management info\n");
      for(;;);
   }

   /* if _info_frame_no is zero then we keep management info in the first
    * frame, else we use the provided frame to keep management info
    */
   if(_info_frame_no == 0) {
      bitmap = (unsigned char *) (_base_frame_no * Machine::PAGE_SIZE);
   } else {
      bitmap = (unsigned char *) (_info_frame_no * Machine::PAGE_SIZE);
   }

   /* Mark all bits in the bitmap
    */
   assert ((_nframes % 8 ) == 0);
   for(i=0; i*8 < _nframes; i++) {
      bitmap[i] = 0xFF;
   }

   /* Mark the first frame as being used if it is being used
    */
   nFreeFrames = _nframes;
   if(_info_frame_no == 0) {
      bitmap[0] = 0x7F;
      nFreeFrames--;
   }
   Console::puts("Initialized Frame Pool\n");
}

unsigned long FramePool::get_frame()
{
   /* Find a frame that is not being used and return its frame index.
    * Mark that frame as being used in the bitmap.
    */
   unsigned char mask;
   unsigned int i = 0;
   unsigned int frame_no = base_frame_no;
   if(nFreeFrames == 0){
      Console::puts("No more frames available\n");
      for(;;);
   }
   while (bitmap[i] == 0x0) {
      i++;
   }
   frame_no += i * 8;
   mask = 0x80;
   while ((mask & bitmap[i]) == 0) {
      mask = mask >> 1;
      frame_no++;
   }
   nFreeFrames--;
   /*
   Console::putui(bitmap[i]);
   Console::putui(mask);
   Console::putui(frame_no);
   Console::putui(i);
   */
   bitmap[i] = bitmap[i] ^ mask;
   
   //Console::puts("Returning Frame ");
   //Console::putui(frame_no);
   //Console::puts("\n");
   
   return (frame_no);
}

void FramePool::mark_inaccessible(unsigned long _frame_no)
{
   assert ((_frame_no >= base_frame_no) && (_frame_no < base_frame_no + nframes));
   unsigned int bitmap_index = (_frame_no - base_frame_no) / 8;
   unsigned char mask = 0x80 >> ((_frame_no - base_frame_no) % 8);
   if((bitmap[bitmap_index] & mask) == 0) {
      Console::puts("Error, Frame being marked inaccessible is already being"
                    "used\n");
      for(;;);
   }
   bitmap[bitmap_index] ^= mask;
   nFreeFrames--;
}

void FramePool::mark_inaccessible(unsigned long _base_frame_no,
                       unsigned long _nframes)
{
   /* Mark all frames in the passed range as being used.
    */
   int i ;
   for(i = _base_frame_no; i < _base_frame_no + _nframes; i++){
      mark_inaccessible(i);
   }
}


void FramePool::release_frame(unsigned long _frame_no)
{
   /* Mark the passed frame is being free once again.
    */
   unsigned int bitmap_index = (_frame_no - base_frame_no) / 8;
   unsigned char mask = 0x80 >> ((_frame_no - base_frame_no) % 8);
   unsigned char byte = bitmap[bitmap_index];
   if((byte & mask) != 0) {
      Console::puts("Error, Frame being released is not being used\n");
      for(;;);
   }
   bitmap[bitmap_index] ^= mask;
   nFreeFrames++;
}
