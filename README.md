# TAR-Archiver
C program for creating an archive, listing elements and extracting them
The Makefile builds the program creating an executable called tarArchiver

The program needs 2 specific files in order to work:

1. A file named "usermap.txt" containing the output of the bash command  cat /etc/passwd
2. A file named "file_ls" containing the output of the bash command  ls -la –time-style=full-iso

The program accepts 4 kinds of commands from the standard input:

1. load archivename  - creates a TAR archive using the informations extracted from the "usermap.txt" and "file_ls" files; "archivename" is the name of the archive that is going to be created; the program will create an archive containing all the files (excluding folders and links) specified in "file_ls"

2. list archivename  - lists all the files contained in the archive, one on each row

3. get archivename filename  - extracts the content of the "filename" file; because this command displays the content of the file at standard output, for binary files, it should be used as follows:  

echo -e “get my_archive my_file.jpg\nquit” | ./tarArchiver > result.jpg

This way, the output of the program is redirected to a file on disk which is equivalent to extracting the file. 
This command leaves the content of the archive unchanged.

4. quit - exits the program







