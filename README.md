# BPE-algorithm
  BPE is an algorithm used for data tokenisation, it is the algorithm used for text tokenisation in many of today's 
state of the art large language models and for lossless data compression.
The input in this context is a string of characters (text) that has what we call vocabulary size, which is the number
of distinct characters that appear in the input. Initially, we generate the list of all consecutive pairs in the input,
the list is then sorted in order of the most common pairs, and then we select a pair (the pair appears in one or more 
locations in the input text) and we replace it with a newly created token, and we keep doing this operation iteratively 
where each time we create a new token we add it to the vocabulary set until we reach a certain desired size.
It is important to keep track of which tokens newly created map to which pairs that we replaced so that the tokenisation 
is lossless, meaning that we can trace the output back to the input and get the original text with no tokenisation.
