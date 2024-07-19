# Spider_CyberSecurity_task2
This contains details of the the previous bonus task and the current task .


Instructions to run the time_module.ko  (previous task)

1)already wrote a c file and used Makefile and converted it into a .ko file

2)now ti insert the module into the kernal , download the .ko file and run the below command
>>>sudo insmod time_module.ko

3)then to see logged messages(diagnostic messages)
>>>dmesg | tail

4)finally to remove the module
>>sudo rmmod time_module.ko
