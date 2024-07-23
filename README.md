# Spider_CyberSecurity_task2
This contains details of the the previous bonus task and the current task .


#Instructions to run the time_module.ko  (previous task)

1)already wrote a c file and used Makefile and converted it into a .ko file

2)now ti insert the module into the kernal , download the .ko file and run the below command
>>>sudo insmod time_module.ko

3)then to see logged messages(diagnostic messages)
>>>dmesg | tail
>>>


4)finally to remove the module
>>sudo rmmod time_module.ko



#Instruction for the second linux module task

have to get syscall table address and then get execve(which executes programs) address and hook it

procedure:

1)since my linux version is 5.18 ( above 5.7) cannot get syscall table address directly . so used kprobes method .
2)fter getting , we first disable write protection and then hook the execve call with a variant of ours then again enable write protection .
3)and when exiting we again put it back to original syscall .


>>>sudo insmod task2.c





