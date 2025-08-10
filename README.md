# The Poor Man LLM

A simple implementation for what it can be called LLM. It has less capabiilities than the autocomplete on your phone, but is a fun weekend project.

This ~200 lines of C89 code uses Markov Chains to create not-random phrases. They don't have much sense tho.

It uses only the standard library, so you should only need a standard C compiler, without downloading or installing anything else.

An example of what is capable of generate:

"The astrologer was attached to take the earth just said my thoughts ran off dissatisfied and distance showing them for masses, to impress the terms of all we've no warning."

# To Do

* Save the model, right now it regenerates everything from scratch in every run.

	Since it implements its own dinamic array, it shoudl be simple enough, like, taking the header of the array, and then the pointer to the data member and save right into a file.

* Add more parameters on the model. Right now uses only one parameter.

	The problem right now, is that the memory requirement grows exponentially with every parameter you add, so 3 or 4 parameters may be the limit before requiring more than 32G of ram.
 
