/*******************************************
 * File: Token.cpp                         *
 * Author: S. Blythe                       *
 * Date: 12/2022                           *
 * PURPOSE: implementation for Token       *
 *******************************************/

/*******************************************
 * Modified by Connor Marshall
 * 1-23-23
 * Computer Systems II Project 1
 * implemented get() and added skipChars()
 * 
*********************************************/

#include "Token.hpp"

#include <fstream>
#include <iomanip>

using namespace std;

// the promised global for string equivalents of TokenType enumeration
string TokStr[]=
{ "ERROR", "EOF_TOK", "NUM_INT", "NUM_REAL", "ADDOP", "MULOP", "ID", "RELOP", "ASSIGNOP", "LPAREN", "RPAREN",  "SEMICOLON",  "LBRACK", "RBRACK", "COMMA", "AND", "OR", "INTEGER", "FLOAT", "WHILE", "IF", "THEN", "ELSE", "VOID", "BEGIN", "END"};

// This is a "list" of the keywords. Note that they are in the same order
//   as found on the TokenType enumaration. 
static string reserved[]={"int" , "float", "while", "if", "then", "else", "void", "begin", "end" };



/******************************************************
 *  just prints out the info describing this Token    *
 *    to specified stream                             *
 *                                                    *
 *   os  - the stream to add the Token to             *
 *                                                    *
 *   returns: the updated stream                      *
 ******************************************************/
ostream&
Token::print(ostream& os) const
{
  os
     << "{ Type:"   << left << setw(10) << TokStr[_type] 
     << " Value:"   << left << setw(10) << _value
     << " Line Number:" << _line_num
     << " }";
  return os;
}

//skips to next non white character in stream. Returns # of new lines
int skipChars(istream &is)
{
  int newLines=0;
  char ch;
  ch = is.get();

  while(ch == '#' || ch == '\n' || ch == ' ' || ch == '\t' || ch == 13)
  {
    if(ch == '#')
    {
      while(ch != '\n' || is.eof())//skip chars until new line or eof
      {
// << " in comment loop. ch :"<< (int)ch << endl;
        ch = is.get();
      }

      //catch an end of file char
      if(is.eof())
      {
        return newLines;
      }
    }

    if(ch == '\n')
    {
      newLines++;
    }

    ch = is.get();
  }

  if(is)
    is.putback(ch);

  return newLines;
}

//automata matrix
static int **DFA = nullptr;



/*****************************************************
 * 
*/
void Token::get(istream &is)
{
  //Build the automata if it isn't built yet
  if(DFA==nullptr)
  {
    DFA = new int*[256];

    for (int r=0; r<=256; r++)
	  {
	    DFA[r]=new int[256];
	    for (int ch=0; ch<256; ch++)
	      DFA[r][ch]=-1;
	  }

    /**************************************************
     * Defining Automata values
     **************************************************/
    
    /*           Start State: 0                 */

    //alpha chars
    for(int i = (int)'A'; i <= (int)'Z'; i++)
    {
      DFA[0][i] = 1;
    }
    for(int i = (int)'a'; i <= (int)'z'; i++)
    {
      DFA[0][i] = 1;
    }

    //Digits
    for(int i = (int)'0'; i <= (int)'9'; i++)
    {
      DFA[0][i] = 2;
    }

    DFA[0][(int)'+'] = 5;
    DFA[0][(int)'-'] = 5;

    DFA[0][(int)'*'] = 6;
    DFA[0][(int)'/'] = 6;

    DFA[0][(int)'<'] = 7;
    DFA[0][(int)'>'] = 7;

    DFA[0][(int)'='] = 9;      
    
    DFA[0][(int)'('] = 10;
    DFA[0][(int)')'] = 11;

    DFA[0][(int)'&'] = 12;

    DFA[0][(int)'|'] = 14;

    DFA[0][(int)';'] = 16;

    DFA[0][(int)'['] = 17;
    DFA[0][(int)']'] = 18;

    DFA[0][(int)','] = 19;

  //Other states

  //from state 1
  for(int i = (int)'A'; i <= (int)'Z'; i++)
    {
      DFA[1][i] = 1;
    }
  for(int i = (int)'a'; i <= (int)'z'; i++)
  {
    DFA[1][i] = 1;
  }
  for(int i = (int)'0'; i <= (int)'9'; i++)
    {
      DFA[1][i] = 1;
    }

  //from state 2
  for(int i = (int)'0'; i <= (int)'9'; i++)
    {
      DFA[2][i] = 2;
    }

  DFA[2][(int)'.'] = 3;

  //from state 3
  for(int i = (int)'0'; i <= (int)'9'; i++)
    {
      DFA[3][i] = 4;
    }

  //from state 4
  for(int i = (int)'0'; i <= (int)'9'; i++)
    {
      DFA[4][i] = 4;
    }

  //from state 7
  DFA[7][(int)'='] = 8;

  //from state 9
  DFA[9][(int)'='] = 8;

  //from state 12
  DFA[12][(int)'&'] = 13;

  //from state 14
  DFA[14][(int)'|'] = 15;
  }

  _value="";
  char ch;

  //skip all white space/comments/new lines to first valid character
  _line_num += skipChars(is); 
  
  if(is.eof())
  {
    _type = EOF_TOK;
    return;
  }

  if(!is) //if the inputstream is empty
  {
    _type = EOF_TOK;
    return;
  }

  int currState=0; //initialized at start state
  int prevState=-1; //stores previous state

  /***********************************************************
   * Lexeme Checking Loop
  */
  while(currState!=-1)
  {
    //read in next character
    ch = is.get();

    //update state
    prevState = currState;
    currState = DFA[currState][(int)ch];

    if(is.eof())
        break;

    //adds character to lexeme if allowed
    if(currState!=-1) _value+=ch;

//cout << "in lexeme loop. char = "<< int(ch) << endl;
  }

  //restore last character to stream
  if(is)
    is.putback(ch);

  //set the type based off the final state
  switch(prevState)
  {
    case 1:
    {
      //keyword check
      if(_value == "if") _type = IF;
      else if(_value == "then") _type = THEN;
      else if(_value == "else") _type = ELSE;
      else if(_value == "while") _type = WHILE;
      else if(_value == "void") _type = VOID;
      else if(_value == "int") _type = INTEGER;
      else if(_value == "float") _type = FLOAT;
      else if(_value == "begin") _type = BEGIN;
      else if(_value == "end") _type = END;

      else _type = ID;
      break;
    }
    case 2:
    {
      _type = NUM_INT;
      break;
    }
    case 4:
    {
      _type = NUM_REAL;
      break;
    } 
    case 5:
    {
      _type = ADDOP;
      break;
    }
    case 6:
    {
      _type = MULOP;
      break;
    }
    case 7:
    {
      _type = RELOP;
      break;
    }
    case 8:
    {
      _type = RELOP;
      break;
    }
    case 9:
    {
      _type = ASSIGNOP;
      break;
    }
    case 10:
    {
      _type = LPAREN;
      break;
    }
    case 11:
    {
      _type = RPAREN;
      break;
    }
    case 13:
    {
      _type = AND;
      break;
    }
    case 15:
    {
      _type = OR;
      break;
    }
    case 16:
    {
      _type = SEMICOLON;
      break;
    }
    case 17:
    {
      _type = LBRACK;
      break;
    }
    case 18:
    {
      _type = RBRACK;
      break;
    }
    case 19:
    {
      _type = COMMA;
      break;
    }
    case 20:
    {
      _type = EOF_TOK;
      break;
    }
    default:
    {
      _type = ERROR;
      break;
    }
  }

}
