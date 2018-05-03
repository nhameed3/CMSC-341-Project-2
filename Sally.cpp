//Author: Nauman Hameed
//Date: 3/7/17
// File: Sally.cpp
// CMSC 341 Spring 2017 Project 2
// Implementation of member functions of Sally Forth interpreter

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <stdexcept>
#include <cstdlib>
using namespace std ;

#include "Sally.h"


// Basic Token constructor. Just assigns values.
//
Token::Token(TokenKind kind, int val, string txt) {
   m_kind = kind ;
   m_value = val ;
   m_text = txt ;
}


// Basic SymTabEntry constructor. Just assigns values.
//
SymTabEntry::SymTabEntry(TokenKind kind, int val, operation_t fptr) {
   m_kind = kind ;
   m_value = val ;
   m_dothis = fptr ;
}


// Constructor for Sally Forth interpreter.
// Adds built-in functions to the symbol table.
//
Sally::Sally(istream& input_stream) :
   istrm(input_stream)  // use member initializer to bind reference
{

   symtab["DUMP"]    =  SymTabEntry(KEYWORD,0,&doDUMP) ;
   symtab["DUP"]    =  SymTabEntry(KEYWORD,0,&doDUP) ;
   symtab["DROP"]    =  SymTabEntry(KEYWORD,0,&doDROP) ;
   symtab["SWAP"]    =  SymTabEntry(KEYWORD,0,&doSWAP) ;
   symtab["ROT"]    =  SymTabEntry(KEYWORD,0,&doROT) ;
   symtab["SET"]    =  SymTabEntry(KEYWORD,0,&doSET) ;


   symtab["<"]    =  SymTabEntry(KEYWORD,0,&doLessThan) ;
   symtab["<="]    =  SymTabEntry(KEYWORD,0,&doLessEqual) ;
   symtab[">"]    =  SymTabEntry(KEYWORD,0,&doGreaterThan) ;
   symtab[">="]    =  SymTabEntry(KEYWORD,0,&doGreaterEqual) ;
   symtab["=="]    =  SymTabEntry(KEYWORD,0,&doEqual) ;
   symtab["!="]    =  SymTabEntry(KEYWORD,0,&doNotEqual) ;
  
   symtab["AND"]    =  SymTabEntry(KEYWORD,0,&doAND) ;
   symtab["OR"]    =  SymTabEntry(KEYWORD,0,&doOR) ;
   symtab["NOT"]    =  SymTabEntry(KEYWORD,0,&doNOT) ;
  
   symtab["IFTHEN"]    =  SymTabEntry(KEYWORD,0,&doIFTHEN) ;
   symtab["ELSE"]    =  SymTabEntry(KEYWORD,0,&doELSE) ;
   symtab["ENDIF"]    =  SymTabEntry(KEYWORD,0,&doENDIF) ;
  
  
   symtab["+"]    =  SymTabEntry(KEYWORD,0,&doPlus) ;
   symtab["!"]    =  SymTabEntry(KEYWORD,0,&doReAssign) ;
   symtab["@"]    =  SymTabEntry(KEYWORD,0,&doAT) ;
   symtab["-"]    =  SymTabEntry(KEYWORD,0,&doMinus) ;
   symtab["*"]    =  SymTabEntry(KEYWORD,0,&doTimes) ;
   symtab["/"]    =  SymTabEntry(KEYWORD,0,&doDivide) ;
   symtab["%"]    =  SymTabEntry(KEYWORD,0,&doMod) ;
   symtab["NEG"]  =  SymTabEntry(KEYWORD,0,&doNEG) ;

   symtab["."]    =  SymTabEntry(KEYWORD,0,&doDot) ;
   symtab["SP"]   =  SymTabEntry(KEYWORD,0,&doSP) ;
   symtab["CR"]   =  SymTabEntry(KEYWORD,0,&doCR) ;
  
   symtab["DO"]    =  SymTabEntry(KEYWORD,0,&doDO) ;
   symtab["UNTIL"]    =  SymTabEntry(KEYWORD,0,&doUNTIL) ;
}


// This function should be called when tkBuffer is empty.
// It adds tokens to tkBuffer.
//
// This function returns when an empty line was entered 
// or if the end-of-file has been reached.
//
// This function returns false when the end-of-file was encountered.
// 
// Processing done by fillBuffer()
//   - detects and ignores comments.
//   - detects string literals and combines as 1 token
//   - detetcs base 10 numbers
// 
//
bool Sally::fillBuffer() {
   string line ;     // single line of input
   int pos ;         // current position in the line
   int len ;         // # of char in current token
   long int n ;      // int value of token
   char *endPtr ;    // used with strtol()


   while(true) {    // keep reading until empty line read or eof

      // get one line from standard in
      //
      getline(istrm, line) ;   

      // if "normal" empty line encountered, return to mainLoop
      //
      if ( line.empty() && !istrm.eof() ) {
         return true ;
      }

      // if eof encountered, return to mainLoop, but say no more
      // input available
      //
      if ( istrm.eof() )  {
         return false ;
      }


      // Process line read

      pos = 0 ;                      // start from the beginning

      // skip over initial spaces & tabs
      //
      while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
         pos++ ; 
      }

      // Keep going until end of line
      //
      while (line[pos] != '\0') {

         // is it a comment?? skip rest of line.
         //
         if (line[pos] == '/' && line[pos+1] == '/') break ;

         // is it a string literal? 
         //
         if (line[pos] == '.' && line[pos+1] == '"') {

            pos += 2 ;  // skip over the ."
            len = 0 ;   // track length of literal

            // look for matching quote or end of line
            //
            while(line[pos+len] != '\0' && line[pos+len] != '"') {
               len++ ;
            }

            // make new string with characters from
            // line[pos] to line[pos+len-1]
            string literal(line,pos,len) ;  // copy from pos for len chars

            // Add to token list
            //
            tkBuffer.push_back( Token(STRING,0,literal) ) ;  

            // Different update if end reached or " found
            //
            if (line[pos+len] == '\0') {
               pos = pos + len ;
            } else {
               pos = pos + len + 1 ;
            }

         } else {  // otherwise "normal" token

            len = 0 ;  // track length of token

            // line[pos] should be an non-white space character
            // look for end of line or space or tab
            //
            while(line[pos+len] != '\0' && line[pos+len] != ' ' && line[pos+len] != '\t') {
               len++ ;
            }

            string literal(line,pos,len) ;   // copy form pos for len chars
            pos = pos + len ;

            // Try to convert to a number
            //
            n = strtol(literal.c_str(), &endPtr, 10) ;

            if (*endPtr == '\0') {
               tkBuffer.push_back( Token(INTEGER,n,literal) ) ;
            } else {
               tkBuffer.push_back( Token(UNKNOWN,0,literal) ) ;
            }
         }

         // skip over trailing spaces & tabs
         //
         while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
            pos++ ; 
         }

      }
   }
}



// Return next token from tkBuffer.
// Call fillBuffer() if needed.
// Checks for end-of-file and throws exception 
//
Token Sally::nextToken() {
      Token tk ;
      bool more = true ;

      while(more && tkBuffer.empty() ) {
         more = fillBuffer() ;
      }

      if ( !more && tkBuffer.empty() ) {
         throw EOProgram("End of Program") ;
      }

      tk = tkBuffer.front() ;
      tkBuffer.pop_front() ;
      return tk ;
}


// The main interpreter loop of the Sally Forth interpreter.
// It gets a token and either push the token onto the parameter
// stack or looks for it in the symbol table.
//
//
void Sally::mainLoop() {

   Token tk ;
   map<string,SymTabEntry>::iterator it ;

   try {
      while( 1 ) {
         tk = nextToken() ;

         if (tk.m_kind == INTEGER || tk.m_kind == STRING) {

            // if INTEGER or STRING just push onto stack
            params.push(tk) ;

         } else { 
            it = symtab.find(tk.m_text) ;
            
            if ( it == symtab.end() )  {   // not in symtab

               params.push(tk) ;

            } else if (it->second.m_kind == KEYWORD)  {

               // invoke the function for this operation
               //
               it->second.m_dothis(this) ;   
               
            } else if (it->second.m_kind == VARIABLE) {

               // variables are pushed as tokens
               //
               tk.m_kind = VARIABLE ;
               params.push(tk) ;

            } else {

               // default action
               //
               params.push(tk) ;

            }
         }
      }

   } catch (EOProgram& e) {

      cerr << "End of Program\n" ;
      if ( params.size() == 0 ) {
         cerr << "Parameter stack empty.\n" ;
      } else {
         cerr << "Parameter stack has " << params.size() << " token(s).\n" ;
      }

   } catch (out_of_range& e) {

      cerr << "Parameter stack underflow??\n" ;

   } catch (...) {

      cerr << "Unexpected exception caught\n" ;

   }
}
// -----------------------------------------------------------------

//Operation adds two tokens on the top of the stack 
void Sally::doPlus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for +.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value + p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Operation subtracts two tokens on the top of the stack 
void Sally::doMinus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for -.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value - p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Operation multiplies two tokens on the top of the stack 
void Sally::doTimes(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for *.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value * p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Operation divides two tokens on the top of the stack 
void Sally::doDivide(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for /.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value / p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Modular operation on two tokens on the top of the stack 
void Sally::doMod(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for %.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value % p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


//----------------------------------------------------------------------------
//Compares two tokens on the top of the stack and determines if one is less than the other
void Sally::doLessThan(Sally *Sptr){
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for <.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value < p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Compares two tokens on the top of the stack and determines if one is less than or equal to the other
void Sally::doLessEqual(Sally *Sptr){
  Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for <=.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value <= p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


//Compares two tokens on the top of the stack and determines if one is greater than the other
void Sally::doGreaterThan(Sally *Sptr){
  Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for >.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value > p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Compares two tokens on the top of the stack and determines if one is greater than or equal to the other
void Sally::doGreaterEqual(Sally *Sptr){
  Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for >=.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value >= p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Compares two tokens on the top of the stack and determines if they are equal
void Sally::doEqual(Sally *Sptr){
  Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for ==.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value == p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


//Compares two tokens on the top of the stack and determines if they are not equal
void Sally::doNotEqual(Sally *Sptr){
  Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for !=.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value != p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//Negates the value of token p
void Sally::doNEG(Sally *Sptr) {
   Token p ;

   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for NEG.") ;
   }
   p = Sptr->params.top() ;
   Sptr->params.pop() ;
   Sptr->params.push( Token(INTEGER, -p.m_value, "") ) ;
}


//-----------------------------------------------------------------------------------

//Performs "&&" comparison operation
void Sally::doAND(Sally *Sptr){
  Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for AND.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
  if(p1.m_value==1 && p2.m_value==1)
    Sptr->params.push( Token(INTEGER, 1, "") ) ;
  else
    Sptr->params.push( Token(INTEGER, 0, "") ) ;
}

//Performs "||" comparison operation
void Sally::doOR(Sally *Sptr){
  Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for OR.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
  if(p1.m_value==1 || p2.m_value==1)
    Sptr->params.push( Token(INTEGER, 1, "") ) ;
  else
    Sptr->params.push( Token(INTEGER, 0, "") ) ;
}

//Changes value from true to false
void Sally::doNOT(Sally *Sptr) {
   Token p ;

   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for NEG.") ;
   }
   p = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer;
   if(p.m_value==1){
    answer = 0;
   }
  
   if(p.m_value==0){
    answer = 1;
   }
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}

//--------------------------------------------------------------------------------------

//Essentially dereferences a token
void Sally::doDot(Sally *Sptr) {
   Token p ;
   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for .") ;
   }

   p = Sptr->params.top() ;
   Sptr->params.pop() ;

   if (p.m_kind == INTEGER) {
      cout << p.m_value ;
   } else {
      cout << p.m_text ;
   }
}

//inserts a space
void Sally::doSP(Sally *Sptr) {
   cout << " " ;
}

//next line
void Sally::doCR(Sally *Sptr) {
   cout << endl ;
}

void Sally::doDUMP(Sally *Sptr) {
   // do whatever for debugging
} 

//Duplicates the top of the stack and pushes the same value on
 void Sally::doDUP(Sally *Sptr){
   Token p1;
   if ( Sptr->params.size() < 1 )
      throw out_of_range("Need one parameters for DUP.") ;
   p1 = Sptr->params.top() ;
   Sptr->params.push(p1) ;
 }
 
//Pops from the top of the stack
  void Sally::doDROP(Sally *Sptr){
   Sptr->params.pop() ;
  }

//Swaps the first two tokens on the stack
  void Sally::doSWAP(Sally *Sptr){
    Token p1, p2;
    if ( Sptr->params.size() < 2 ) 
      throw out_of_range("Need two parameters for SWAP.") ;
    p1 = Sptr->params.top() ;
    Sptr->params.pop() ;
    p2 = Sptr->params.top() ;
    Sptr->params.pop() ;
    Sptr->params.push(p1) ;
    Sptr->params.push(p2) ;
  }

//Rotates the first three tokens on the stack
   void Sally::doROT(Sally *Sptr){
     Token p1, p2, p3;
     if ( Sptr->params.size() < 3 ) 
      throw out_of_range("Need three parameters for ROT.") ;
     p1 = Sptr->params.top() ;
     Sptr->params.pop() ;
     p2 = Sptr->params.top() ;
     Sptr->params.pop() ;
     p3 = Sptr->params.top() ;
     Sptr->params.pop() ;
     Sptr->params.push(p2) ;
     Sptr->params.push(p1) ;
     Sptr->params.push(p3) ;
   }
//------------------------------------------------------------------------
   
//Creats a variable and sets it equal to a value, if that variable isnt already created
    void Sally::doSET(Sally *Sptr){
      Token p1, p2;
      if ( Sptr->params.size() < 2 ) 
        throw out_of_range("Need two parameters for SET.") ;
      p1 = Sptr->params.top() ;
      Sptr->params.pop() ;
      p2 = Sptr->params.top() ;
      Sptr->params.pop() ;
      
      //iterates through the symbol table to search for the variable
      map<string,SymTabEntry>::iterator it ;
      it = Sptr->symtab.find(p1.m_text);
      if( it != Sptr->symtab.end() ){
        cout << "ERROR VARIABLE NAME IN USE" << endl;
        return;
      }
      if( it == Sptr->symtab.end() ){
        Sptr->symtab[p1.m_text]=SymTabEntry(VARIABLE,p2.m_value,NULL);
      }
    }

//Obtains the value of a varible
  void Sally::doAT(Sally *Sptr){
    Token p1;
    if ( Sptr->params.size() < 1 )
      throw out_of_range("Need one parameter for @.") ;
    map<string,SymTabEntry>::iterator it;
    p1 = Sptr->params.top();
    Sptr->params.pop();
    it = Sptr->symtab.find(p1.m_text);
    if( it == Sptr->symtab.end() ){
        cout << "ERROR VARIABLE NOT FOUND" << endl;
        return;
    }
    else{
      p1.m_value = Sptr->symtab.find(p1.m_text)->second.m_value;
      int answer = p1.m_value;
      Sptr->params.push( Token(INTEGER, answer, "") ) ;
    }
  } 

//Sets a variable that already exists to a new value
  void Sally::doReAssign(Sally *Sptr){
      Token p1, p2;
      if ( Sptr->params.size() < 2 )
        throw out_of_range("Need two parameters for !.") ;
      p1 = Sptr->params.top() ;
      Sptr->params.pop() ;
      p2 = Sptr->params.top() ;
      Sptr->params.pop() ;
      
      map<string,SymTabEntry>::iterator it ;
      it = Sptr->symtab.find(p1.m_text);
      if( it != Sptr->symtab.end() ){
        Sptr->symtab[p1.m_text]=SymTabEntry(VARIABLE,p2.m_value,NULL);
      }
      if( it == Sptr->symtab.end() ){
        cout << "ERROR VARIABLE NOT FOUND" << endl;
        return;
      }
  }

//------------------------------------------------------------------------------
 //Checks for nestes ifthens and executes  
  void Sally::doIFTHEN(Sally *Sptr){
    Token p;
    int count=1;
    
    p = Sptr->params.top() ;
    Sptr->params.pop();
    
    if( p.m_value==1){
      return;
    }
    
    if(p.m_value == 0){
      
      while(count > 0){
        
        if(Sptr->nextToken().m_text=="IFTHEN")
          count++;
        if(Sptr->nextToken().m_text=="ELSE")
          count--;
        
      }
      
    }
    
  }

   //pushes onward towards ENDIF
   void Sally::doELSE(Sally *Sptr){
     while(Sptr->nextToken().m_text!="ENDIF"){
       Sptr->nextToken();
     }
   }
   
//Does nothing
   void Sally::doENDIF(Sally *Sptr){
     return;
   }

//----------------------------------------------------------------------------------------

//Records all tokens into a list and stops when it hits the token UNTIL
  void Sally::doDO(Sally *Sptr){
    Token p = Sptr->nextToken();
    while(p.m_text!="UNTIL"){
      Sptr->recorder.push_back(p);
      p = Sptr->nextToken();
    }
    //Have to push the token UNTIL onto the stack in order for UNTIL to be called
    Sptr->recorder.push_back(p);   
    
    //pushes all the tokens into the buffer
    list<Token>::iterator it=Sptr->recorder.end();
    it--;
    for( ; ; --it){
       Sptr->tkBuffer.push_front(*it);
       if(it == Sptr->recorder.begin()){ 
         break;
       }
    }
  }

//If the boolen is true then returns, if false then pushes into the front of the tkbuffer
  void Sally::doUNTIL(Sally *Sptr){
    Token p;
    p = Sptr->params.top() ;
    Sptr->params.pop();   
    if( p.m_value==1){
      return;
    } 
   
    //pushes all the tokens into the buffer
    list<Token>::iterator it=Sptr->recorder.end();
    it--;
    for( ; ; --it){
       Sptr->tkBuffer.push_front(*it);
       if(it == Sptr->recorder.begin()){ 
         break;
       }
    }
    
    /*
    list<Token>::iterator it=Sptr->recorder.end();
    it--;
    while(it != Sptr->recorder.begin()){
      --it;
       Sptr->tkBuffer.push_front(*it);
    }*/
    
    /*
    list<Token>::iterator it;
    for (list<Token>::iterator it=Sptr->recorder.begin() ; it != Sptr->recorder.end(); ++it){
      Sptr->tkBuffer.push_back(*it);
    }*/
    
  }

























