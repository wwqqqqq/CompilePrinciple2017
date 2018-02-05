##coding structure
1. Using a recursion function and a global variable to do the job.
    - The function nfa has two parameters: the current state and the next character pointer. (**nfa(int state, char \*str)**)
    - Function nfa is a recursion function.
    - To print the required substring, using a global variable **char** \***start**.
    - The variable start is a pointer that point to the starting character of the legal substring. 
2. The transition rule of the nfa: (current function is **nfa(current state,str)**)
    - current state = 0
        * First, check whether the next character is '\0'.  
          Character '\0' indicates the end of string. If it is detected when the state is 0, the function should end. 
        * If the character is neither 'a' nor 'b', then the function moves to the next character. (**nfa(0,str+1)**)
        * The state changes to both 1 and 3. (**nfa(1,str);nfa(3,str);**)
    - current state = 1
        * If the next character is not 'a', then return.
        * If the next character is 'a', then set the globla variable start to the parameter str, and state changes to 2. (**start=str; nfa(2,str+1);**)
    - current state = 3
        * Similar to state 1.
    - current state = 2
        * If the next character is 'a', then moves to the next character. (**nfa(2,str+1)**)
        * If the character is other than 'a', then str-1 is point to the last character of the legal substring. 
          Using pointer start and str to print the substring, and return to state 0. (**nfa(0,str)**)
    - current state = 4
        * Similar to state 2.
3. The main function reads two parameters **int argc** and **char** \*\***argv** from the command line, and uses argv[1] as the second parameter of function nfa.  
   The starting state of function nfa is 0.
##Usage
* The source code is in file nfa.c under this directory and the executable file is named as nfa.
* The input string is read from the command line.
* USAGE: ./nfa <the given string\>
* e.g.   
  input : ./nfa aaab  
  output: aaa  
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;b
##The identifying process of string "aaab"
1. The main function get the string "aaab" and calls function nfa(0,str1).**str1** is the pointer that points to the first character in the string. (\*str1 = 'a')
2. executing nfa(0,str1)  
   The first character is not '\0'. So function nfa(0,str1) calls two functions nfa(1,str1) and nfa(3,str1).
3. executing nfa(1,str1)  
   \*str1 = 'a', so the next state is 2. Set global variable **start** = str1 and call nfa(2,str2). **str2** is the pointer that points to the second character in the string. (\*str2 = 'a')
4. executing nfa(2,str2)  
   \*str2 = 'a', so the next state is still 2. Call nfa(2,str3). **str3** is the pointer that points to the third character in the string. (\*str3 = 'a')
5. executing nfa(2,str3)  
   \*str3 = 'a', so the next state is still 2. Call nfa(2,str4). **str4** is the pointer that points to the forth character in the string. (\*str4 = 'b')
6. executing nfa(2,str4)  
   \*str4 = 'b', so print the substring and then call function nfa(0,str4). start == str1 (the first character of string). The second parameter str4 points to the forth character of string.  
       for(;start<str4;start++) putchar(*start);  
   So the first identified substring is **"aaa"**.  
7. executing nfa(0,str4)  
   Since \*str4 is not '\0', calls both nfa(1,str4) and nfa(3,str4).
8. executing nfa(1,str4)
   \*str4 is not 'a', so directly return.
9. executing nfa(2,str4)  
   \*str4 = 'b', so the next state is 4. Set **start** = str4 and call nfa(4,str5). **str5 = str4 +1. It points to the fifth character in the string (which is actually '\0')
10. executing nfa(4,str5)  
   \*str5 is not 'b', so print the substring and then call function nfa(0,str5). start == str4. So the printed identified substring is **"b"**.
11. executing nfa(0,str5)  
   str5 points to character '\0', which indicates the end of string. Return.
12. executing nfa(3,str1)  
   \*str1 is not 'b', so return. Ending of the whole procedure.