# miniFS
<img width="566" alt="image" src="https://github.com/c4nkn/miniFS/assets/56227236/e36f2343-bd13-4d04-9f9c-5da6f8a6c4cb">
<br></br>

👀 Yep, I did file system simulation in disk environment using C. It's similar to Unix-like file systems cuz it uses INodes.
It allows users to perform basic file system operations.
- Create, read, write, and delete files
- Create and delete directories
- List directory contents
- Change current directory
- Display file or directory information
Umm, currently this project isn't fully completed. There might be bugs, errors.

### Requirements
- GCC *(if you use makefile, if you will not use it any other C compiler is ok)*

### Installation
```
git clone https://github.com/c4nkn/miniFS.git
cd miniFS
make
./miniFS
```

### Usage
```
miniFS> mkdir testDir
miniFS> create testDir/testFile
miniFS> write testDir/testFile text (currently it doesnt support blank space)
miniFS> read testDir/testFile
miniFS> info testDir/testFile
miniFS> rmdir testDir
```

### Contributing
🥳 Contributions are welcome! You can contact me for anything, anytime.
