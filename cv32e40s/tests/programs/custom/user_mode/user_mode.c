// Copyright 2021 OpenHW Group
// Copyright 2021 Silicon Labs, Inc.
//
// Licensed under the Solderpad Hardware Licence, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://solderpad.org/licenses/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier:Apache-2.0 WITH SHL-2.0

#include <stdio.h>
#include <stdlib.h>

static int enter_usermode;
static int enter_machinemode;

void handle_ecall(void){
  
  volatile unsigned int readd;

  // clear mstatus.mpp (bits 12:11)
  __asm__ volatile("csrr %0, 0x300" : "=r"(readd));
  printf("handle_ecall: mstatus.mpp = 0x%x\n", (readd >> 11) & 0x3);

  
  if (enter_usermode) {
    printf("Entering user mode\n");
    // clear mstatus.mpp (bits 12:11)
    __asm__ volatile("li t0, 0x00000000");
    enter_usermode = 0;
  }
  else if (enter_machinemode) {
    printf("Exiting user mode\n");
    // set mstatus.mpp (bits 12:11)
    __asm__ volatile("li t0, 0x00001800");
    enter_machinemode = 0;
  }

  __asm__ volatile("csrw 0x300, t0");

  end_handler_incr_mepc();
}

int main(void) {

  volatile unsigned int readd;
  
  enter_usermode = 0;
  enter_machinemode = 0;

  printf("\nHello, User mode test!\n\n");

  // clear mcounteren
  __asm__ volatile("li t0, 0x00000000");
  __asm__ volatile("csrw 0x306, t0");
  __asm__ volatile("csrr %0, 0x306" : "=r"(readd));
  printf("mcounteren = 0x%x\n", readd);

  // Go to user mode
  enter_usermode = 1;
  asm volatile("ecall");
    
  // check mstatus.mpp
  asm volatile("ecall");

  // This should not be allowed unless mcounteren[0] is set.
  __asm__ volatile("csrr %0, 0xC00" : "=r"(readd));
  printf("readd = 0x%x\n", readd);
  
  // Go to machine mode
  enter_machinemode = 1;
  asm volatile("ecall");

  printf("\nGoodbye, user mode test!\n\n");
  return EXIT_SUCCESS;
}
