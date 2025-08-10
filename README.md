# The Poor Man LLM

A simple implementation for what it can be called LLM. It has less capabiilities than the autocomplete on your phone, but is a fun weekend project.

This ~200 lines of C89 code uses Markov Chains to create not-random phrases. They don't have much sense tho.

It uses only the standard library, so you should only need a standard C compiler, without downloading or installing anything else.

An example of what is capable of generate:

"The astrologer was attached to take the earth just said my thoughts ran off dissatisfied and distance showing them for masses, to impress the terms of all we've no warning."

My next goal is to increase the number of parameters, but the Markov Chain have two limitations compared with more advanced LLM:

* The memory requirements grows exponentially with the number of parameters added.
* The more parameters you add, the more "plagiaristic" the text becomes. So there is a point where the generated text can be a copy and paste from the training material. But here's the thing: Maybe there are cases that you want that behavior, like when you train the data with code. In such case you want the exact match. If you are writing letters or you are trying to talk about philosophy or psychology, maybe not.

# To Do

* Add more parameters on the model. Right now uses only one parameter.

	The problem right now, is that the memory requirement grows exponentially with every parameter you add, so 3 or 4 parameters may be the limit before requiring more than 32G of ram.
 
