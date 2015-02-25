#include <iomanip>
#include <assert.h>
#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <thread>
#include <future>
#include <memory>
#include <cstdlib>
#include <algorithm>

using namespace std;


struct bf{

  enum class Instructions{
    PLUS,
    MINUS,
    INC,
    DEC,
    IN,
    OUT,
    OPEN,
    CLOSE,
    ZERO_CELL,
    INVALID
  };

  struct Ins{
    Instructions val;
    int count;

    Ins( Instructions v ): val( v ), count(1) {}
    Ins( Instructions v , int c): val( v ), count(c) {}

    string toString(){
      string ret = "";
      switch( val ){
        case Instructions::PLUS: ret+= "PLUS"; break;
        case Instructions::MINUS: ret+= "MINUS"; break;
        case Instructions::INC: ret+= "INC"; break;
        case Instructions::DEC: ret+= "DEC"; break;
        case Instructions::IN: ret+= "IN"; break;
        case Instructions::OUT: ret+= "OUT"; break;
        case Instructions::OPEN: ret+= "OPEN"; break;
        case Instructions::CLOSE: ret+= "CLOSE"; break;
        case Instructions::ZERO_CELL: ret += "ZERO_CELL";break;
        case Instructions::INVALID: ret+= "INVALID";break;
      }
      ret+= " "+ to_string( count );

      return ret;
    }

    bool operator==( Ins const that ){
      if ( that.val == val && that.count == count ){
        return true;
      } else {
        return false;
      }
    }

    bool operator!= ( Ins const that ){
      return ! ( (*this) == that );
    }

  };

  

  vector<uint8_t> tape;
  vector<long int> loopStack;
  vector<Ins> instructions;

  long int ip=0;

  
  long int loc=0;
  bool die = false;
  ostream &os;
  istream &is;

  bf(ostream &o, istream &i ): 
      tape(1,0),
      ip(0),
      loc(0),
      die(false),
      os(o),
      is(i)  {}

  void compile( const string &s ){
    long int i=0;
    for( uint8_t c : s ){
      switch ( c ){
        case '+': instructions.push_back(Instructions::PLUS); break;
        case '-': instructions.push_back(Instructions::MINUS); break;
        case '>': instructions.push_back(Instructions::INC); break;
        case '<': instructions.push_back(Instructions::DEC); break;
        case '.': instructions.push_back(Instructions::OUT); break;
        case ',': instructions.push_back(Instructions::IN); break;
        case '[': instructions.push_back(Instructions::OPEN); break;
        case ']': instructions.push_back(Instructions::CLOSE); break;
        default: i--; break;
      }
      ++i;
    }
  }

  void dumpInstructions(){
    cout<<"Instruction Dump"<<endl;
    for( size_t i=0; i<instructions.size(); i ++ ){
      cout<<setw( 4 )<< i <<" "<< instructions[i].toString() << endl;
    }
  }

  void collapseRuns(){
    auto colapse = [](Instructions i ){
      switch ( i ){
        case Instructions::PLUS:
        case Instructions::MINUS:
        case Instructions::DEC:
        case Instructions::INC:
          return true;
        default:
          return false;
      }
    };

    vector<Ins> temp;
    temp.reserve( instructions.size() );

    Ins last = instructions.front();
    last.count = 0;

    for( auto i : instructions ){
      if( ! colapse( i.val ) ){
        temp.push_back( last );
        last = i;
        continue;
      }

      if( i.val == last.val ){
        last.count += i.count ;
      } else {
        temp.push_back( last );
        last = i;
      }
    }

    temp.push_back( last );

    instructions = move( temp );
  }
  void find_zero_cell_opertunities(){
    decltype( instructions ) temp;
    temp.reserve( instructions.size() );

    auto here = instructions.begin();
    auto where = instructions.begin();
    auto end = instructions.end();

    Ins opn( Instructions::OPEN,1 );
    Ins min( Instructions::MINUS,1 );
    Ins pls( Instructions::PLUS,1 );
    Ins cls( Instructions::CLOSE,1 );
    while( true ){
      where = std::find( here, end,  opn );
      temp.insert( temp.end() , here, where );
      if( where == end ) break;
      if( 
          ( where + 1) == end || ( *(where+1) != min && *(where+1) != pls ) || 
          ( where + 2) == end || *(where+2) != cls ){
        where += 1;
      } else {
        //else we have something that can be replaced
        temp.push_back( Ins(Instructions::ZERO_CELL, 1 ) );
        where += 2;
      }
      here = where;
    }
  }

  void removeZeroCount(){
    decltype( instructions ) temp;
    temp.reserve( instructions.size() );

    for ( auto i : instructions ){
      if( i.count > 0 ){
        temp.push_back( i );
      }
    }
    instructions = move( temp );

  }

  //optimize the bf instructions
  void optimize(){
    if( instructions.size() == 0 ){ return; }

    collapseRuns();
    find_zero_cell_opertunities();
    removeZeroCount();
  }

  void run(){
    if( instructions.empty() ){
      return;
    }
    while( !die ){
      switch(instructions[ip].val){
        case Instructions::PLUS : plus( instructions[ip].count ) ; break;
        case Instructions::MINUS : minus( instructions[ip].count ) ; break;
        case Instructions::INC : inc( instructions[ip].count ) ; break;
        case Instructions::DEC : dec( instructions[ip].count ) ; break;
        case Instructions::OUT : out( instructions[ip].count ) ; break;
        case Instructions::IN : in( instructions[ip].count ) ; break;
        case Instructions::OPEN : open( instructions[ip].count ) ; break;
        case Instructions::CLOSE : close( instructions[ip].count ) ; break;
        case Instructions::ZERO_CELL : zero_cell() ; break;
        default :
          cerr<<"Error in run"<<endl;
          die = true;
      }
      if( ip >= (int)instructions.size() ) die=true;
    }
  }

  void zero_cell(){
    tape[loc]=0;
    ++ip;
  }

  void inc( int count ){
    loc += count;
    if( loc >=  (long int) tape.size() ){ tape.resize( loc+1 ); }
    ++ip;
  }

  void dec( int count ){
    loc -= count;
    if( loc < 0 ){ die = true ; }
    ++ip;
  }

  void plus( int count ){
    tape[loc] += count;
    ++ip;
  }

  void minus( int count ){
    tape[loc] -= count;
    ++ip;
  }

  void out( int count ){
    assert( count == 1 );
    os<<tape[loc];
    os.flush();
    ++ip;
  }

  void in( int count ){
    assert( count == 1 );
    is>>tape[loc] ;
    if( is.eof() ){
      die=true;
    }
    ++ip;
  }

  void open( int count ){
    assert( count == 1 );
    if( tape[loc] != 0 ){
      loopStack.push_back(ip);
      ++ip;
    } else {
      try{
        int level = 1;
        while(true){
          ++ip;
          if( instructions.at(ip).val == Instructions::CLOSE ){
            -- level ;
          } else if ( Instructions::OPEN == instructions.at(ip).val ){
            ++level;
          }

          if( level == 0 ){
            ++ip;
            return;
          }
        }
      } catch ( const out_of_range &e ){
        cerr<<"No closing of loop found: "<< e.what()<<endl;
        die=true;
      }
    }
  }

  void close( int count ){
    assert( count == 1 );
    ip = loopStack.back();
    loopStack.pop_back();
  }

};

void printUsage(){
  cout<<"bfc - a brainfuck intrepreter"<<endl;
  cout<<"Usage: bfc [-h] [--help] [filenames]"<<endl;
  cout<<endl;
  cout<<"if called with no filename bfc will read from stdin until eof is "<<endl;
  cout<<"   found then start the bf program it read. stdin will be avaliable"<<endl;
  cout<<"   for new input to the program."<<endl;
  cout<<"if a filename is given it will be intrepreted by bfc"<<endl;
  cout<<endl;
  cout<<"if multiple files are given all are run with output going to $(FILENAME).bfout"<<endl;
  cout<<"   stdout is not avaliable to these programs"<<endl;
  cout<<endl;
  cout<<"both -h and --help will print this message"<<endl;
  cout<<endl;
}

vector< string > parseArgs( int argc , char ** argv ){
  vector<string> ret;
  ret.reserve( argc );
  if( argc == 1 ){ return ret; }
  for( auto ii=argv+1 ; ii<argv+argc ; ii++){
    ret.push_back(*ii);
  }

  for( const auto &s : ret ){
    if ( s=="--help" || s == "-h" ){
      printUsage();
      exit(0);
    }
  }
  return ret;
}

string streamAsString( istream &is ){
  string ret;
  vector<string> lines;
  size_t len=0;

  string line;
  do{ 
    getline( is ,line);
    if( ! is.eof()  ){
      len+= line.size();
      lines.emplace_back( line );
    }
  }while( is.good() ) ;

  ret.reserve(len);
  for ( auto &s : lines ){
    ret += s;
  }

  return ret;
}

istringstream ss;
void doMulti ( const vector<string> &args ) {
  vector<unique_ptr<ifstream> > files;
  vector<unique_ptr<ofstream> > ofiles;
  vector< bf > bfs;
  vector< future<void> > fut ;

  files.reserve( args.size() );
  ofiles.reserve( args.size() );
  bfs.reserve( args.size() );
  fut.reserve( args.size() ) ;

  for( const auto &s : args ){
    files.emplace_back( make_unique<ifstream>( s ) );
    ofiles.emplace_back( make_unique<ofstream>( (s+".bfout").c_str() ) );
    string lines = streamAsString( *files.back() );
    bf b ( *ofiles.back() , ss );
    b.compile( lines );

    fut.push_back( async( 
          std::launch::async , 
          [](bf bb){ bb.run(); } ,b 
          )  );
  }

  for( auto &i : fut ){
    i.get();
  }
}

void  doSingle ( string fname ){
  bf b(cout,cin);
  ifstream is( fname );
  string line = streamAsString( is );

  b.compile( line );
  b.optimize();
  //b.dumpInstructions();
  b.run();
}

void doNone( ){
  bf b(cout,cin);
  string line = streamAsString( cin );

  b.compile( line );
  b.run();
}

int main(int argc , char ** argv ){

  vector<string> args = parseArgs(argc, argv);

  string line;
  vector<string> lines;

  if( args.size() == 0 ){
    doNone();
  } else if( args.size() == 1 ){
    doSingle( args[0] );
  } else {
    doMulti( args );
  }

  return 0;
}
