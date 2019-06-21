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

## Language problem
If the pipe data contains non-ascii characters, they may not be properly shown. Use 'chcp 65001' to change the codepage to UTF8 because 'cnote' asusumes inputs are endcoded in UTF8.

```
>chcp 65001
Active code page: 65001
>dir | cnote -W
```
