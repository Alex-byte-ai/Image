Procedure Branch(x:integer);

Var
  i,n, nn, px, py : integer;
  h : real;

Begin
  h:=Heading;
  Position(px,py);
  If x>6 then nn:=8 else nn:=75;
  For n:=1 to nn do Begin
    If (x>3) and (n>22) and (n<342)
    then Pen(x, 255, 100+n*2, 75-n)
    else Pen(x, 100, 255-x*18, 0);
    Forward(x);
    If x mod 2 = 0 then Right(n+random(n div 2)+8)
    else Left(n+random(n div 2)+7);
    If (n=8) and (x>0) then Branch(x-1);
    If (n=6) and (x>0) then Branch(x-2);
    Delay({0} {1} 0);
  end;
  SetHeading(h);
  PenUp;
  SetPosition(px,py);
  PenDown;
end;

Begin
  Brush({0} {1} 0, 50, 100, 250);
  Fill(0,0);
  Hide;
  PenUp;
  SetPosition(600,600);
  PenDown;
  Branch(13);
end;