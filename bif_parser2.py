import re

def bif_parse_read(filename):
    parsed_dictionary = {};
    """Parses the .bif file with the
    given name (exclude the extension from the argument)
    and produces a python file with create_graph() and create_bbn() functions
    to return the network. The name of the module is returned.
    The bbn/factor_graph objects will have the filename as their model name."""

    # Setting up I/O
    module_name = filename+'_bn'
    infile = open(filename+'.bif')
    infile.readline()
    infile.readline()

    # Regex patterns for parsing
    variable_pattern = re.compile(r"  type discrete \[ \d+ \] \{ (.+) \};\s*")
    prior_probability_pattern_1 = re.compile(
        r"probability \( ([^|]+) \) \{\s*")
    prior_probability_pattern_2 = re.compile(r"  table (.+);\s*")
    conditional_probability_pattern_1 = (
        re.compile(r"probability \( (.+) \| (.+) \) \{\s*"))
    conditional_probability_pattern_2 = re.compile(r"  \((.+)\) (.+);\s*")

    variables = {};  # domains
    distributions = [];
    # For every line in the file
    while True:
        line = infile.readline()
        # End of file
        if not line:
            break
        # Variable declaration
        if line.startswith("variable"):
            match = variable_pattern.match(infile.readline())
            # Extract domain and place into dictionary
            if match:
                var = line[9:-3];
                for mch in match.group(1).split(", "):
                    if var not in variables:
                        variables[var] = [];
                    variables[var].append( mch )
            else:
                raise Exception("Unrecognised variable declaration:\n" + line)
            infile.readline()
        # Probability distribution
        elif line.startswith("probability"):
            match = prior_probability_pattern_1.match(line)
            if match:
                # Prior probabilities
                variable = match.group(1)
                function_name = "f_" + variable
                #functions.append(function_name)
                line = infile.readline()
                match = prior_probability_pattern_2.match(line)
                infile.readline()  # }
                dicti = {};
                for var in variables[variable]:
                    dicti[var] = 1;
                distributions.append( ['Prior', variable, dicti ] );
            else:
                match = conditional_probability_pattern_1.match(line)
                if match:
                    # Conditional probabilities
                    variable = match.group(1)
                    function_name = "f_" + variable
                    #functions.append(function_name)
                    given = match.group(2).split(", ");
                    dictionary = {}
                    all_values = [];
                    # Iterate through the conditional probability table
                    while True:
                        line = infile.readline()  # line of the CPT
                        if line == '}\n':
                            break
                        match = conditional_probability_pattern_2.match(line)
                        given_values = match.group(1).split(", ")
                        for value, prob in zip(
                                variables[variable],
                                map(str, match.group(2).split(", "))):
                            dictionary[tuple([value]+given_values )] = 1;
                    distributions.append( [ 'Conditional', variable, given, dictionary ] );
                else:
                    raise Exception(
                        "Unrecognised probability declaration:\n" + line)
        
    return [variables, distributions];