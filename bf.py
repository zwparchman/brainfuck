#!/usr/bin/python

import sys
import itertools


def group( s ):
    gg = [(len(list(group)),name) for name, group in itertools.groupby(s)]
    out = []
    for i in gg:
        if( i[1] in ',.[]'):
            '''expand back out'''
            out+= i[0] *[( 1 ,i[1] )]
            continue

        out+=[i]

    return out

def aprint( a , b ):
    if a != b:
        print a
    assert( a == b )

aprint( group( ">><<[[]]" ) , [(2,'>'),(2,'<'),(1,'['),(1,'['),(1,']'),(1,']')] )

def stripNonBf( s ):
    ret = ""
    for c in s:
        if c in '[]<>-+,.':
            ret += c

    return ret

#runs extended brainfuck that is compiled by this program from a file every run
class Ebf :
    dp=0
    ip=0
    loopStack=[]
    instructions=[]
    tape = [0]
    die=False

    def __init__( self , f ):
        full = f.read()
        full = stripNonBf( full )
        full = group( full )

        self.instructions = full

    def add( self , count ):
        dp = self.dp
        tape = self.tape
        count = count % 255
        tape[dp] = ( tape[dp] + count ) % 255
        self.ip += 1

    def sub( self, count ):
        count = count % 255
        self.tape[ self.dp] = ( self.tape[ self.dp] - count )
        if self.tape[ self.dp] < 0 :
            self.tape[ self.dp] = -self.tape[ self.dp]
        self.ip += 1

    def dec( self, count ):
        self.dp -= count
        if self.dp < 0:
            self.die = True
        self.ip += 1

    def inc( self, count ):
        self.dp += count
        if self.dp >= len( self.tape ):
            self.tape += [0]*count
        self.ip += 1

    def out( self , noUse ):
        del noUse
        c = self.tape[ self.dp ]
        try :
            c = chr ( c )
        except Exception as e:
            pass
        else:
            sys.stdout.write( c )
            sys.stdout.flush()

        self.ip += 1

    def _in( self , noUse ):
        del noUse
        c = sys.stdin.read( 1 )
        if len( c ) == 0 :
            self.die = True
            return
        self.tape[self.dp] = ord(c)
        self.ip += 1

    def _open( self , noUse ):
        del noUse

        if self.tape[self.dp] != 0:
            self.loopStack.append( self.ip )
            self.ip += 1
        else:
            level = 1
            while True:
                try:
                    self.ip += 1
                    if self.instructions[self.ip][1] == ']' :
                        level -= 1
                    elif self.instructions[self.ip] [1]== '[':
                        level += 1

                    if level == 0 :
                        self.ip += 1
                        return
                except IndexError  as e:
                    print "out of bounds instruction. ip=",self.ip
                    print e
                    die = True;
                    return

    def close( self , noUse ):
        del noUse

        self.ip = self.loopStack[-1]
        self.loopStack.pop()

    ftable = {
            '+' : add,
            '-' : sub,
            '<' : dec,
            '>' : inc,
            '.' : out,
            ',' : _in,
            '[' : _open,
            ']' : close
            }

    def dump( self ):
        print "ins:",self.instructions[self.ip],"ip:",self.ip,"loopStack:",self.loopStack,"tape[dp]:",self.tape[ self.dp] ,"dp",self.dp

    def run(self):
        while not self.die:
            ins = self.instructions[ self.ip ]

            if ins[1] == '.' :
                pass

            self.ftable [ ins[1] ] (self,ins[0] )

            if self.ip >= len( self.instructions ):
                self.die = True




def main():
    sys.stdin.softspace = False
    e = Ebf( file( "./hanoi.b", "rb" ) )
    e.run()
    #print e.instructions

if __name__ == "__main__":
    main()

