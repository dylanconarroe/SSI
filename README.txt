CSC 360 Assignment P1

Feature 1:
	
Implemented Shell Prompt that displays username@hostname: current_working_directory >

The current working directory is displayed as the absolute path and updates when directory changes.


Feature 2:

The SSI allows for foreground execution using fork(), execvp(), and waitpid(), such as ls, echo, pwd, etc.


Feature 3:

The SSI allows for user to change directories with built in cd command using chdir() such as cd ~, cd ., cd A, etc.


Feature 4:

EOF handling, allows user to press Ctrl-D to exit SSI and return to original shell.


Feature 5:

SSI supports background execution using the bg command such as bg sleep 20 and bg ping -c -i 2 1.1.1.1


Feature 6:

SSI supports background job listing using bglist to display the PID and command of the currently running background jobs followed by the number of jobs. 


Feature 7:

SSI reports when background jobs terminate using waitpid() and WNOHANG.


Feature 8:

SSI has signal handling, allowing user to terminate a process using Ctrl-C, returning user to the SSI prompt. If not process is running, Ctrl-C displays a new prompt instead of terminating SSI shell. 


Testing:

Program was tested on linux.csc.uvic.ca using various commands and using p1_self_evaluation. 

