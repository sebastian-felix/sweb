/**
 * @file ArchThreads.cpp
 *
 */

#include "ArchThreads.h"
#include "ArchCommon.h"
#include "ArchMemory.h"
#include "kprintf.h"
#include "paging-definitions.h"
#include "offsets.h"
#include "Thread.h"
#include "Scheduler.h"

void ArchThreads::initialise()
{
  currentThreadInfo = (ArchThreadInfo*) new uint8[sizeof(ArchThreadInfo)];
}

extern "C" uint32 kernel_page_directory_start;

void ArchThreads::setAddressSpace(Thread *thread, ArchMemory& arch_memory)
{
//  thread->kernel_arch_thread_info_->cr3 = arch_memory.page_dir_page_ * PAGE_SIZE;
//  if (thread->user_arch_thread_info_)
//    thread->user_arch_thread_info_->cr3 = arch_memory.page_dir_page_ * PAGE_SIZE;
}

uint32 ArchThreads::getPageDirectory(Thread *thread)
{
//  return thread->kernel_arch_thread_info_->cr3 / PAGE_SIZE;
  //TODO: should be the same for now, have to return only one
}


void ArchThreads::createThreadInfosKernelThread(ArchThreadInfo *&info, pointer start_function, pointer stack)
{
  info = (ArchThreadInfo*)new uint8[sizeof(ArchThreadInfo)];
  ArchCommon::bzero((pointer)info,sizeof(ArchThreadInfo));
  pointer pageDirectory = VIRTUAL_TO_PHYSICAL_BOOT(((pointer)&kernel_page_directory_start));

  info->pc = start_function;
  info->cpsr = 0x60000013;
  info->sp = stack & 0xFFFFFFF0;
}

void ArchThreads::createThreadInfosUserspaceThread(ArchThreadInfo *&info, pointer start_function, pointer user_stack, pointer kernel_stack)
{
  info = (ArchThreadInfo*)new uint8[sizeof(ArchThreadInfo)];
  ArchCommon::bzero((pointer)info,sizeof(ArchThreadInfo));
  pointer pageDirectory = VIRTUAL_TO_PHYSICAL_BOOT(((pointer)&kernel_page_directory_start));

  info->pc = start_function;
  info->cpsr = 0x60000010;
  info->sp = user_stack & 0xFFFFFFF0;

}

void ArchThreads::cleanupThreadInfos(ArchThreadInfo *&info)
{
  //avoid NULL-Pointer
  if (info)
    delete info;
}

extern "C" void halt();
void ArchThreads::yield()
{
  halt();
}

extern "C" uint32 arch_TestAndSet(uint32 new_value, uint32 *lock);
uint32 ArchThreads::testSetLock(uint32 &lock, uint32 new_value)
{
  return arch_TestAndSet(new_value, &lock);
}

uint32 ArchThreads::atomic_add(uint32 &value, int32 increment)
{
  int32 ret=increment;
  /*__asm__ __volatile__(
  "lock; xadd %0, %1;"
  :"=a" (ret), "=m" (value)
  :"a" (ret)
  :);*/
  return ret;
}

int32 ArchThreads::atomic_add(int32 &value, int32 increment)
{
  return (int32) ArchThreads::atomic_add((uint32 &) value, increment);
}

void ArchThreads::printThreadRegisters(Thread *thread, uint32 userspace_registers)
{
  ArchThreadInfo *info = userspace_registers?thread->user_arch_thread_info_:thread->kernel_arch_thread_info_;
  if (!info)
  {
    kprintfd("Error, this thread's archthreadinfo is 0 for use userspace regs: %d\n",userspace_registers);
    return;
  }
  kprintfd("%sThread: %10x, info: %10x -- pc: %10x  sp: %10x  lr: %10x  cpsr: %10x -- r0:%10x r1:%10x r2:%10x r3:%10x r4:%10x r5:%10x r6:%10x r7:%10x r8:%10x r9:%10x r10:%10x r11:%10x r12:%10x\n",
           userspace_registers?"  User":"Kernel",thread,info,info->pc,info->sp,info->lr,info->cpsr,info->r0,info->r1,info->r2,info->r3,info->r4,info->r5,info->r6,info->r7,info->r8,info->r9,info->r10,info->r11,info->r12);

}