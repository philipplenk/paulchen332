# paulchen332

A simple UCI chess engine

![Engine Logo]("https://codemetas.de/images/pchess_logo.png" "Isn't it lovely?")

You can learn more about it in [this dedicated blog article](https://codemetas.de/2020/11/18/The-Royal-Game.html)

# Building

paulchen332 depends on two tiny header only libraries of mine, [ptl](https://github.com/philipplenk/ptl) and [pcl](https://github.com/philipplenk/pcl). They are included in this repository as git submodules. 
They are not automatically initialized when you do a simple clone, so after

    git clone https://github.com/philipplenk/paulchen332
	
you have to run the following commands to get them pulled in:

    git submodule init
    git submodule update

Alternatively, this command should work for doing both in one step:

    git clone --recurse-submodules https://github.com/philipplenk/paulchen332

This repository contains a very simple Makefile of a quality which is commonly refered to as horrendous. I was really lazy with that and it contains no information at all about dependencies.
As a lot of my code resides in headers, this implies a partial rebuild after some changes will likely break in funny and unforseeable ways. As such, a complete rebuild, using simple 

    make clean && make
    
is recommended. This will leave an executable named **paulchen332** in the toplevel directory.

# Notes

I am usually extremly shy, so for me to publish any code at all can be considered a minor miracle. As such, as mentioned in [my first article on it](https://codemetas.de/2020/11/18/The-Royal-Game.html),
this repository contains everything that is needed to build the engine, but nothing else. There are a number of tests and code used for tuning which is just in too horrifying a state to expose my dear readers to and I have stripped a lot of self-deprecating comments. I do beg your forgiveness.
I'd also advice not to take any of the code shown here, without the corresponding blog articles which will explain my reasoning, as great advice on how to go about programming a chess engine.

This is simply a first public release, which undoubtedly contains many a bug, inefficiency and just plain stupidity, significantly curbing its playing strength ;-) 

