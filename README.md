# The Poor Man LLM

A simple implementation for what it can be called LLM. It has less capabiilities than the autocomplete on your phone, but is a fun weekend project.

This ~1k lines of C89 code uses N-Order Markov Chains to create not-random phrases. They don't have much sense tho.

It uses only the standard library, so you should only need a standard C compiler, without downloading or installing anything else.

An example of what is capable of generate:

"The astrologer was attached to take the earth just said my thoughts ran off dissatisfied and distance showing them for masses, to impress the terms of all we've no warning."

My next goal is to increase the number of parameters, but the Markov Chain have two limitations compared with more advanced LLM:

* The memory requirements grows exponentially with the number of parameters added.
* The more parameters you add, the more "plagiaristic" the text becomes. So there is a point where the generated text can be a copy and paste from the training material. But here's the thing: Maybe there are cases that you want that behavior, like when you train the data with code. In such case you want the exact match. If you are writing letters or you are trying to talk about philosophy or psychology, maybe not.

# How to compile

You need linux, because pthreads, but maybe you can compile on windows id you use Cygwin or WSL. Who knows. Typing this on the root folder will create the executable.

```make build_release```

Besides that, no other libraries are needed. No compiler extensions neither.

**WARNING** if you try to run and you don't have the training model (you will not have it the first time) it will use the file "libro.txt" to create the model. Since this file is pretty big (have several books stitch together) the process can take around 5 hours. It uses multithread to process it, but the merge of the results into a single graph takes most of the time. You can try to download a smaller book from Project Gutemberg website in utf-8 format. The book "Moby Dick" it will take 15 or 20 minutes. Different books will create diferent "feeling" of the generated answers.

# How To Add Parameters

The number of parameters are controlled by the defined value NODE_NUM_PARAM on the file node_t.h. The more parameters you add, the more memory you need to run the program, and the more "plagiaristic" the generated phrase is. But maybe you want plagiarism you you are training the program with your own code. Who knows.




