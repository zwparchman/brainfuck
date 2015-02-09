import unsigned 
import ropes

type 
  Instruction = enum
    PLUS, MINUS,
    INC, DEC,
    IN, OUT,
    OPEN, CLOSE

  bf = object of RootObj
    ip : int
    dp : int
    tape : seq[uint8]
    instructions : seq[Instruction]
    loopStack : seq[int]
    die : bool

proc bf_init() : bf =
  var b : bf

  b.loopStack= @[]
  b.instructions= @[]
  b.tape= @[]
  b.tape.add( 0 )
  return b

proc compile( b : var bf , s : string ) =
  for c in s:
    case c
    of '+' : b.instructions.add( Instruction.PLUS )
    of '-' : b.instructions.add( Instruction.MINUS )
    of '>' : b.instructions.add( Instruction.INC )
    of '<' : b.instructions.add( Instruction.DEC )
    of ',' : b.instructions.add( Instruction.IN )
    of '.' : b.instructions.add( Instruction.OUT )
    of '[' : b.instructions.add( Instruction.OPEN )
    of ']' : b.instructions.add( Instruction.CLOSE )
    else : discard
  return

proc add( b: var bf ) =
  b.tape[b.dp] += 1
  b.ip += 1

proc minus( b: var bf ) =
  b.tape[b.dp] -= 1
  b.ip += 1

proc inc( b: var bf ) =
  b.dp += 1
  if b.tape.len == b.dp :
    b.tape.add( 0 )
  b.ip += 1

proc dec( b: var bf ) =
  if 0 == b.dp :
    b.die = true
  b.dp -= 1
  b.ip += 1

proc bf_in( b: var bf ) =
  b.ip += 1
  var c = readChar( stdin )
  var bb = cast[uint8]( c )
  b.tape[ b.dp ] = bb

proc bf_out( b: var bf ) =
  var c = cast[char]( b.tape[b.dp] )
  write(  stdout , c  )
  flushFile( stdin )
  b.ip += 1

proc open( b: var bf ) =
  if (b.tape[ b.dp ] != 0'u8 ) :
    b.loopStack.add( b.ip )
  else:
    var level = 1
    while ( true ):
      b.ip += 1;
      if b.ip > b.instructions.len : b.die=true; return

      if( b.instructions[b.ip] == Instruction.CLOSE ):
        level -= 1
      elif b.instructions[b.ip] == Instruction.OPEN :
        level += 1

      if level == 0 :
        b.ip += 1
        return
  b.ip += 1

proc close( b: var bf ) =
  b.ip = b.loopStack.pop()

proc run( b: var bf ) =
  while true :
    if b.die or b.ip == len( b.instructions ) : break
    case b.instructions[ b.ip ] 
    of Instruction.PLUS : b.add
    of Instruction.MINUS : b.minus 
    of Instruction.INC : b.inc 
    of Instruction.DEC : b.dec 
    of Instruction.IN : b.bf_in
    of Instruction.OUT : b.bf_out  
    of Instruction.OPEN : b.open 
    of Instruction.CLOSE : b.close 
    else : echo "Error" 


proc get() : string =
  var r = rope("")
  var f = open("mantrelblot.b")
  defer: close(f)

  try:
    while( true ):
      var line = f.readLine()
      r = r & line
  except IOError:
    discard

  return $r


proc main () =

  var b = bf_init()

  var ss =  get() 

  b.compile( ss )
  b.run

main()
echo ""
