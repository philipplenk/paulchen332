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

(also: the makefile is what is commonly refered to as crap. and the lazy kind.)

(because it is far more complicated and convoluted than it ought to be. But it was fun to write xD)

(i am rather shy, so for me to publish any code at all is a minor miracle)
