# Project Title
Simple Shell

## Description

Simple Unix Shell built with C.


## Getting Started

### Dependencies
* Unix Shell

### Executing program

* run system call
```
make
```
* run the script using user input
```
./myshell
```

## Notes & To Do
Input & output redirection does not work (ex. cat < file1 > file2)
Output redirection overwrites the files instead of flushing old content 
(ex: ls > file1cat file2 > file1) file1 contains results of ls and content of file2, it should only contain file2. 
fixing warnings

## Authors

[@Mohamed Ayman](https://www.linkedin.com/in/mohayman3600/)


## Version History

* 0.1
    * Initial Release

## License

This project is licensed under the [MIT] License - see the LICENSE.md file for details
