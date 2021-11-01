/* Calculator with a "paper tape" that exports to a file */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>


/* Remove carriage return and/or line feed characters from a string.
   YOU ARE ALLOWED TO COPY THIS FUNCTION. */

void remove_crlf(char *s)
{
    char *t = s + strlen(s);

    // t begins at the null sentinel at the end of s.

    t--;

    /* t is now at the last character of s - unless s didn't contain any characters, in 
       which case, t is now *BEFORE* s.  We have to keep checking for that. */

    /* We repeat until EITHER t slides to the left of s, OR we find a character that is 
       not a line feed (\n) or a carriage return (\r). */

    while ((t >= s) && (*t == '\n' || *t == '\r'))
    {
        *t = '\0'; // Clobber the character t is pointing at.
        t--;       // Decrement t.
    }
}



typedef struct Calculator
{
  int hist_idx;
  int hist_lenght;
  double memory[10];
  double *history;
} Calculator;



/* Function Prototypes */

// General functions
int isNumber(char *buffer);
int step1(Calculator *calc, FILE *ofp, char *input);
int step2(Calculator *calc, FILE *ofp);

// Unary operations
int unary_operations(Calculator *calc, FILE *ofp, char *buffer);

// Binary operations
int binary_operations(Calculator *calc, FILE *ofp, char *buffer);
int store(Calculator *calc, FILE *ofp);

// Special operations
void clear_all(Calculator *calc);
int get_last_entry(Calculator *calc, FILE *ofp);
int recall(Calculator *calc, FILE *ofp);


/* Constructor. Initializes struct Calculator */

Calculator *create_mycalculator()
{
  Calculator *ptr = malloc(sizeof(Calculator));
  ptr->hist_lenght = 10;
  ptr->hist_idx = -1;  // No elements. First element will have hist_idx = 0, hence history[hist_idx]

  // Per instructions, start all store variables at zero
  for (int i = 0; i < 10; i++) {
    ptr->memory[i] = 0;
  }

  ptr->history = malloc(sizeof(double) * ptr->hist_lenght);

  return ptr;
}


/* Deconstructor. Free memory space */

void free_mycalculator(Calculator *calc)
{
  free(calc->history);
  free(calc);
}



/* MASTER OF CEREMONIES */

int main(void) {

  Calculator *calc = create_mycalculator();
  FILE *ofp = fopen("tape.txt", "w"); 

  char buffer[128];
  int flag;

  do
  {
      printf("\nEnter a number.\n> ");
      fgets(buffer, 127, stdin);
      remove_crlf(buffer);

      flag = step1(calc, ofp, buffer);

      if(flag == -1) {              // Quit "q" program. Break do-while loop and fclose ofp and free memory
        fprintf(ofp, "q\n");       
        break;
      }
      else if(flag == 0)           // User enters "c" or "ca". Continue while loop (ask for step 1 number)
        continue;
      else if(flag == -2) {        // History Function "b" reaches the end of history
        printf("> Error. End of history.\n");     
        fprintf(ofp, "Error. End of history.\n"); 
        continue;
      }
      else if(flag == -3) {        // User entered invalid operation or number
        printf("> Error\n");      
        fprintf(ofp, "Error\n");
        continue;
      }
      else if(flag == 1)          // Store funcion from step2
        continue;

  } while (1);

  fclose(ofp);
  free_mycalculator(calc);

  return 0;
}


/* Step 1 Function. Accept a floating-point number or 'c', 'ca', 'b', and 'q' */

int step1(Calculator *calc, FILE *ofp, char *input)
{
  double number;

  if (!strcmp(input, "pi"))
    number = M_PI;
  else if (!strcmp(input, "e"))
    number = M_E;
  else if(isNumber(input))
    number = atof(input); 
  
  /* Special Operations Step 1*/
  else if(!strcmp(input, "c"))
  {
    printf("> 0\n");
    fprintf(ofp, "c\n0\n");
    return 0; // Print > 0 to terminal and tape file. Also print operation name to tape file
  }
  else if(!strcmp(input, "ca"))
  {
    fprintf(ofp, "ca\n");
    clear_all(calc);
    return 0;
  }
  else if (!strcmp(input, "q"))
    return -1;
  else if (!strcmp(input, "b"))
    return get_last_entry(calc, ofp);

  else 
    return -3; // If not special operation ('c', 'ca', 'b', and 'q') nor a number -> Error. Return to main Step1 


  calc->hist_idx++;
  calc->history[calc->hist_idx] = number;
  fprintf(ofp, "%23.17lf\n", number);   
  return step2(calc, ofp);
  
}



/* Step 2 Function: This function executes if a number is returned from Step 1 function */

int step2(Calculator *calc, FILE *ofp)
{
  char buffer[128];
  int flag;

  while(1)
  {
    // hist_idx starts at -1. When hist_idx = 0, history array has 1 element. 
    if(calc->hist_idx + 1 >= calc->hist_lenght)  
    {
      calc->hist_lenght *= 2;  // Initially was set to 10, now it is 20 and so on
      calc->history = (double *)realloc(calc->history, sizeof(double) * calc->hist_lenght);
    }

    // printf("\n\n");

    // // Prints Memory Array
    // for (int i = 0; i < 10; i++)
    //   printf("memory[%d] = %f\n", i, calc->memory[i]);

    // printf("\n\n");

    // Prints History Array
    // for(int i = 0; i <= calc->hist_idx; i++)
    //   printf("History_Array[%d] = %23.17lf\n", i, calc->history[i]);

    printf("\nOperation:\n> ");
    fgets(buffer, 127, stdin);
    remove_crlf(buffer);


    /* Binary Operations */
    if(!strcmp(buffer, "+") || !strcmp(buffer, "-") || !strcmp(buffer, "x") ||
       !strcmp(buffer, "/") || !strcmp(buffer, "^"))
    {
      flag = binary_operations(calc, ofp, buffer);
      if(flag == 1)
        continue;
      else
        return flag;
    }

    /* Store Operation Step 2. It's binary */
    else if(!strcmp(buffer, "sto"))
    {
      fprintf(ofp, "sto\n");
      return flag = store(calc, ofp);
    }   


    /* Unary operations */
    else if(!strcmp(buffer, "sin") || !strcmp(buffer, "cos") || !strcmp(buffer, "tan") ||
            !strcmp(buffer, "arcsin") || !strcmp(buffer, "arccos") || !strcmp(buffer, "arctan") ||
            !strcmp(buffer, "root") || !strcmp(buffer, "abs") || !strcmp(buffer, "inv") ||
            !strcmp(buffer, "log") || !strcmp(buffer, "log2") || !strcmp(buffer, "rcl"))
    {
      flag = unary_operations(calc, ofp, buffer);
      if(flag == 1)
        continue;
      else
        return flag;
    }


    /* Special Operations Step 2*/
    else if(!strcmp(buffer, "c"))
    {
      printf("> 0\n");
      fprintf(ofp, "c\n0\n");
      return 0; 
    }

    else if(!strcmp(buffer, "ca"))
    {
      fprintf(ofp, "ca\n");
      clear_all(calc);
      return 0;
    }

    else if(!strcmp(buffer, "q"))
      return -1;

    else if(!strcmp(buffer, "b"))
    {
      if(get_last_entry(calc, ofp) == 1)
        continue;
      else      
        return -2; // To go back to step1 
    } 
   

    return -3;  // If not an allowed operation, return to main Step1

  } // while() loop

} // Step 2 Function



/* Unary Math Operations. Does not include recall "recall" function */
int unary_operations(Calculator *calc, FILE *ofp, char *op_buffer)
{
  double result;
  double prevNum = calc->history[calc->hist_idx];
  int max = INT_MAX;
  int min = INT_MIN;

  fprintf(ofp, "%s\n", op_buffer);

  if(!strcmp(op_buffer, "abs"))
    result = fabs(prevNum);
  else if(!strcmp(op_buffer, "sin"))
    result = sin(prevNum);
  else if(!strcmp(op_buffer, "cos"))
    result = cos(prevNum);
  else if(!strcmp(op_buffer, "tan") && ((result = tan(prevNum)) > max || result < min ))
    return -3; // tan() function overflows when it's undefined (odd multiples of pi/2) 
  else if(!strcmp(op_buffer, "arcsin") && isnan(result = asin(prevNum)))  
    return -3; // asin() function returns nan when argument is out of domain [-1, 1] 
  else if(!strcmp(op_buffer, "arccos") && isnan(result = acos(prevNum)))
    return -3; // acos() function returns nan when argument is out of domain [-1, 1]
  else if(!strcmp(op_buffer, "arctan"))
    result = atan(prevNum);
  else if(!strcmp(op_buffer, "inv") && isinf(result = 1 / prevNum))
    return -3;

  // sqrt(), log10(), log2() functions return nan when argument is < 0
  //         log10(), log2() functions return inf when argument is = 0

  else if(!strcmp(op_buffer, "root") && isnan(result = sqrt(prevNum)))
    return -3;
  else if(!strcmp(op_buffer, "log") && (isnan(result = log10(prevNum)) || isinf(result)))
    return -3;
  else if(!strcmp(op_buffer, "log2") && (isnan(result = log2(prevNum)) || isinf(result)))
    return -3;
  else if(!strcmp(op_buffer, "rcl"))
    return recall(calc, ofp);


  calc->history[calc->hist_idx + 1] = result;
  calc->hist_idx++;

  printf("= %23.17lf\n\n", calc->history[calc->hist_idx]);
  fprintf(ofp, "=\n%23.17lf\n", calc->history[calc->hist_idx]);

  return 1;
}



/* Binary Math Operations + - x /. Does not include store 'sto' function */
int binary_operations(Calculator *calc, FILE *ofp, char *op_buffer)
{
  char buffer[128];
  float num, res;

  fprintf(ofp, "%s\n", op_buffer);  
 
  while (1)
  {
    printf("\nEnter number:\n> ");
    fgets(buffer, 127, stdin);
    remove_crlf(buffer);

    /* Special Operations */
    if(strcmp(buffer, "c") == 0)
    {
      printf("> 0\n");
      fprintf(ofp, "c\n0\n");
      return 0;
    }
    else if(strcmp(buffer, "ca") == 0)
    {
      fprintf(ofp, "ca\n");
      clear_all(calc);
      return 0;
    }
    else if(strcmp(buffer, "q") == 0)
      return -1;
    else if(strcmp(buffer, "b") == 0)
    {
      if(get_last_entry(calc, ofp) == 1)
      {
        num = calc->history[calc->hist_idx];        
        break;
      }
      else      
        return -2; 
    }
    
    /* Allowed numbers */
    else if(strcmp(buffer, "pi") == 0)
    {
      num = M_PI;
      calc->hist_idx++;
      break;
    }
    else if(strcmp(buffer, "e") == 0)
    {
      num = M_E;
      calc->hist_idx++;
      break;
    }
    else if(isNumber(buffer))
    {
      num = atof(buffer);
      calc->hist_idx++;
      break;
    }

    /* If not special operation ('c', 'ca', 'b', and 'q') nor a number -> Error. Return to main Step1 */
    return -3;
  }
  

  calc->history[calc->hist_idx] = num;

  // if buffer == b, the get_last_entry() function fprints the 2nd number to the tape file
  if(strcmp(buffer, "b") != 0) 
    fprintf(ofp, "%23.17lf\n", calc->history[calc->hist_idx]);


  if(!strcmp(op_buffer, "+"))
    res = calc->history[calc->hist_idx - 1] + calc->history[calc->hist_idx];
  else if(!strcmp(op_buffer, "-"))
    res = calc->history[calc->hist_idx - 1] - calc->history[calc->hist_idx];
  else if(!strcmp(op_buffer, "x"))
    res = calc->history[calc->hist_idx - 1] * calc->history[calc->hist_idx];
  else if(!strcmp(op_buffer, "/") && isinf(res = calc->history[calc->hist_idx - 1] / calc->history[calc->hist_idx]))
    return -3;
  else if(!strcmp(op_buffer, "^") && isinf(res = pow(calc->history[calc->hist_idx - 1], calc->history[calc->hist_idx])))
    return -3;


  calc->history[calc->hist_idx + 1] = res;
  calc->hist_idx++;  

  printf("\n= %23.17lf\n", calc->history[calc->hist_idx]);
  fprintf(ofp, "=\n%23.17lf\n", calc->history[calc->hist_idx]);
  return 1;
}


/* Store "sto" Function */
int store(Calculator *calc, FILE *ofp)
{
  char buffer[128];
  int num;

  printf("\nEnter a digit 0-9 to store the previous number: > ");
  fgets(buffer, 127, stdin);
  remove_crlf(buffer);

  if(isNumber(buffer))
  {
    num = atoi(buffer); // number entered is a number

    if(num >= 0 && num <= 9)  
    {
      calc->memory[num] = calc->history[calc->hist_idx];
      calc->hist_idx++;
      calc->history[calc->hist_idx] = num;  // Save number from 0-9 in history
      fprintf(ofp, "%d\nStored in memory %d\n", num, num); // Print to tape file digit 0-9. Otherwise, print Error
      printf("Stored in memory %d\n", num);
      return 1;  // OK
    }
    else 
      return -3; // Error. Number entered is not 0-9. Go back to step1
  }
  else
    return -3; // Error. Not a valid number. Go back to step1
}


/* Function (C)lear (A)ll store variables */
void clear_all(Calculator *calc)
{
  for (int i = 0; i < 10; i++)
  {
    calc->memory[i] = 0;
  }
}


/* Function (R)e(C)a(L)l */
int recall(Calculator *calc, FILE *ofp)
{
  double prevNum = calc->history[calc->hist_idx];

  if(prevNum >= 0 && prevNum <= 9)  
  {
    printf("Recalled memory %d\n> %23.17lf\n", (int)prevNum, calc->memory[(int)prevNum]);
    fprintf(ofp, "Recalled memory %d\n%23.17lf\n", (int)prevNum, calc->memory[(int)prevNum]);

    calc->hist_idx++;
    calc->history[calc->hist_idx] = calc->memory[(int)prevNum];
    return 1; // OK
  }
  else
    return -3; // Error. Number entered is not 0-9. Go back to step1
}



/* Function "b" history */
int get_last_entry(Calculator *calc, FILE *ofp)
{
  char buffer[64];
  int prev = calc->hist_idx;

  if(prev < 0) // First op is b and there is no numbers stored yet
    return -2;

  do
  { 
    fprintf(ofp, "b\n");
    printf("> %23.17lf\n", calc->history[prev]);      // printf last number/result entered
    fprintf(ofp, "%23.17lf\n", calc->history[prev]);

    prev--;

    printf("\nType 'Enter' to use prev number/result or 'b' to backtrack: > ");
    fgets(buffer, 127, stdin);
    remove_crlf(buffer);  

  } while (strcmp(buffer, "b") == 0 && prev >= 0);
    

  if(strcmp(buffer, "b") == 0) // user enter b but (prev < 0). End of history, no more #s to display
  {
    fprintf(ofp, "b\n");
    return -2;
  }

  calc->hist_idx++; // Go to next hist_idx and copy number from prev

  calc->history[calc->hist_idx] = calc->history[prev + 1];
  printf("> %23.17lf\n", calc->history[calc->hist_idx]);
  fprintf(ofp, "%23.17lf\n", calc->history[calc->hist_idx]); 

  return 1;
} 



/* Function determines if input from stdin is a floating-point number. 
  Returns 1 if it's a floating-point number, 0 otherwise */

int isNumber(char *buffer)
{
  int digit_count = 0;
  int point_count = 0;

  int str_length = strlen(buffer);
  // printf("String length: %d\n", str_length);

  if(isdigit(buffer[0]) || buffer[0] == '-' || buffer[0] == '+') // First character is a digit 0-9, negative or plus sign
  {
    digit_count = 1;

    for (int i = 1; i < str_length; i++)
    {
      if(isdigit(buffer[i])) // isdigit() from 0-9
        digit_count++;
      else if (buffer[i] == '.')
        point_count++;       
    }

    if(point_count <= 1 && (point_count + digit_count) == str_length)
      return 1; // It's a valid number
    else
      return  0; // String starts with digit (0-9) OR + or - symbols, but it is not a valid number (It could have more than one floating point or non-digit characters)
  }
  else
    return 0; // First character of the string is not a digit nor a minus symbol
}