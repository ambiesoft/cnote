# cnote

## Usage
```
> dir | cnote
```
This launches a notepad with text of the output of 'dir'.

```
> somecommand | cnote -W
```
'-W' will replace \r or \n to \r\n.
As previous version of Notepad.exe can not render \n properly, this will fix the problem.

## Powershell character encoding problem 
Looks like the redirection of powershell command always convert the data with US-ASCII, this application can not show the strings properly if it has been badly encoded.
