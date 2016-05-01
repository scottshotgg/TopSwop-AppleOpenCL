# TopSwop-AppleOpenCL

TopSwap takes arrays of size n, fills them with 1 through n+1, and then randomizes them. After this is completed, it takes the value of first element of the array, goes to the array element of that value, and symmetrically swaps everything in between. It does this until it gets a 1 in front, at which it considers this array TopSwapped. It does this for a specified amount of times and then records the highest amount. Being on OpenCL, this is done on hundreds of GPU threads.
