# mypdfsearch
Searching program for words(or extracting text) from the pages of a pdf file.

mypdfsearch version 1.0.0

Usage:
	
mypdfsearch Options

Options(at least one of):
	
	'-h, --help' (optional)	Print help message and exit.
	
	'-v, --version' (optional)	Print version info and exit.
	
	'-e, --extracttextfrom' (optional)	Extract text from specified file and exit.
		Arguments for option '-e, --extracttextfrom': a string indicating the pdf file
		                                              from which to extract the text.
	
	'-f, --frompage' (optional)	Start search(or extract text) from pagenum specified.
		Arguments for option '-f, --frompage': an integer indicating the start page of the pdf file
		                                       from which to start the search (or extract the text).
	
	'-t', '--topage' (optional)	Stop search(or extract text) to pagenum specified
		Arguments for option '-t, --topage': an integer indicating the final page of the pdf file
		                                     in which to stop the search (or extract the text).
	
	'-p', '--path' (optional)	Specifies the input file.
		Arguments for option '-p, --path': a string indicating the directory on which the program will search
		                                   for the pdf files to be searched.
		                                   The program will search for the words indicated in the '--words' option
		                                   in the directory indicated and in its subdirectories.
	
	'-w', '--words' (optional)	Words to search
		Arguments for option '-w, --words': A string enclosed in double quotation marks ",
		                                    indicating the words to search for (separated by a space).
	
	'-o', '--outputfile' (optional)	Output file: specifies the output file
	Arguments for option '-o, --outputfile': The name of the text file where the search results will be stored
	                                         (or store the extracted text from the pdf file).
	                                         

--------------------------------------------------- USAGE EXAMPLES-------------------------------------------------

1)
	mypdfsearch --path="/myhome/myfiles" --words = "hello world"

		look for the words "hello" and "world" in all the pdf files contained in
		"/myhome/myfiles" directory and in all its subdirectories.

2)
	mypdfsearch --extracttextfrom="/myhome/myfiles/myfile.pdf" --frompage=5 --topage=21
	
		extract the text from the "myfile.pdf" file, from page 5 to page 21.
	
3)
	mypdfsearch --path="/myhome/myfiles" --words = "hello world" --outputfile=myresults.txt
	
		look for the words "hello" and "world" in all the pdf files contained in
		"/myhome/myfiles" directory and in all its subdirectories
		and stores the results in the "myresults.txt" file instead of showing them on the screen.
