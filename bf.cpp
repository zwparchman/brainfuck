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
    CLOSE
  };


  vector<uint8_t> tape;
  vector<long int> loopStack;
  vector<Instructions> instructions;

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

  void run(){
    if( instructions.empty() ){
      return;
    }
    while( !die ){
      switch(instructions[ip]){
        case Instructions::PLUS : plus() ; break;
        case Instructions::MINUS : minus() ; break;
        case Instructions::INC : inc() ; break;
        case Instructions::DEC : dec() ; break;
        case Instructions::OUT : out() ; break;
        case Instructions::IN : in() ; break;
        case Instructions::OPEN : open() ; break;
        case Instructions::CLOSE : close() ; break;
        default :
          cerr<<"Error in run"<<endl;
      }
      if( ip >= (int)instructions.size() ) die=true;
    }
  }

  void inc(){
    ++loc ;
    if( loc >=  (long int) tape.size() ){ tape.push_back(0); }
    ++ip;
  }

  void dec(){
    --loc ;
    if( loc < 0 ){ die = true ; }
    ++ip;
  }

  void plus(){
    ++tape[loc];
    ++ip;
  }

  void minus(){
    --tape[loc];
    ++ip;
  }

  void out(){
    os<<tape[loc];
    os.flush();
    ++ip;
  }

  void in(){
    is>>tape[loc] ;
    if( is.eof() ){
      die=true;
    }
    ++ip;
  }

  void open(){
    if( tape[loc] != 0 ){
      loopStack.push_back(ip);
      ++ip;
    } else {
      try{
        int level = 1;
        while(true){
          ++ip;
          if( instructions.at(ip) == Instructions::CLOSE ){
            -- level ;
          } else if ( Instructions::OPEN == instructions.at(ip) ){
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

  void close(){
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
