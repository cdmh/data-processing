https://code.google.com/p/naive-bayes-classifier/
=================================================


= Naive Bayes Classifier =

"A Bayes classifier is a simple probabilistic classifier based on applying Bayes' theorem (from Bayesian statistics)" -- Wikipedia (http://en.wikipedia.org/wiki/Naive_Bayes_classifier)

It is mainly used in artificial intelligence.

== How to use the Naive Bayes Classifier ==

The Naive Bayes Classifier needs data to function. The data is contained in a simple text file. Floats are used to represent every data. 

In my example, the bayesianClassifierValues.txt contains the following values :

1.0 2.0 1.0
2.0 1.0 1.0
1.0 2.0 0.0
0.0 0.0 0.0

---------Beware, it won't work if there is an empty line at the end of the file-------

The input or situation data is contained into the n - 1 first columns. In this case, it is the first and second column.
The output observed depending on the situation is the last column.

For example, these data contain situations observed by a Boss in a 2D scroller video game.

 - The first column is the relative position of his opponent on the X axis.
 - The second column is the relative position of his opponent on the Y axis.
 - The last column (the output) is a boolean representing if the opponent is hurt or not.

=== Domains ===

A Bayes classifier needs discrete values to work properly. For each column, a domain has to be associated. 
Each of these domains contains the minimum value, maximum value and the number of values between them.

For example, the domain for the first column (the relative position of the opponent on the X axis) is defined like this :

Domain(0.0, 2.0, 3)

The minimum value is 0.0 and maximum value is 2.0. The number of values between them are 3. Hence, the conversion is defined like this :

Discrete value = ((value - min) * number of values) / (max - min)

0 = ((0.5 - 0.0) * 3 / (2.0 - 0.0)
1 = ((0.67 - 0.0) * 3 / (2.0 - 0.0)
1 = ((1.32 - 0.0) * 3 / (2.0 - 0.0)
2 = ((1.34 - 0.0) * 3 / (2.0 - 0.0)

Approximatively, the domains for the first column are these :

[0, 0.666[ = 0
[0.666, 1.333[ = 1
[1.333, 2.0[ = 2

Changing the number of values between the minimum and maximum can affect greatly the results. Test with multiple values for better results.

=== BayesianClassifier ===

The BayesianClassifier is capable of calculating the most probable output depending of the input. 
It is possible to add new raw data at runtime and have a better probabilistic classifier.

=== ActionClassifier ===

The ActionClassifier is capable of calculating the best action to perform to obtain a certain output.
It is possible to add new raw data at runtime and have a better probabilistic classifier.

It uses the BayesianClassifier but it has 1 more column. The data is still contained in a simple text file.

In my example, the actionClassifierValues.txt contains the following values :

1.0 2.0 1.0 1.0
2.0 1.0 1.0 1.0
1.0 2.0 0.0 0.0
0.0 0.0 0.0 0.0

---------Beware, it won't work if there is an empty line at the end of the file-------

The input or situation data is contained into the n - 2 first columns. In this case, it is the first and second column.
The output observed depending on the situation is the n - 1 column or in this case, the third column. 
The action done in this situation (input) obtaining the output is the last column. 

For example, these data contain situations observed by a Boss in a 2D scroller video game.

 - The first column is the relative position of his opponent on the X axis.
 - The second column is the relative position of his opponent on the Y axis.
 - The third column (the output) is a boolean representing if the opponent is hurt or not.
 - The last column (the action) is a number representing the action done by the Boss. A C++ enumeration is recommended.
 