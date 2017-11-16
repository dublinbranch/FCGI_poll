Standard FCGI uses select so the whole application CAN NOT have more than 1024 open file
Also has a pair of syscall of too much

We replace SELECT with POLL, marked the SOCKET as BLOCKING (I think) and... I think is all

Do not check the edit history for the relevant change... compare the os_unix file with the original one