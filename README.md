# lexer_generator

I've been experimenting with language design lately and I got fed up with writing a different lexer every time I wanted to try creating a new language, so I decided to create a lexer generator (I call it a pre-lexer) that takes in a set of token rules and generates a lexer.

Here's the general rundown:

**#token** creates a simple literal pre-lexer token. It's not limited to one character, and can be any length.

**#container** creates either a string or a vector (this is written in C++) of strings that can later be used by the lexer and the pre-lexer.

**#pattern** creates pre-lexer tokens based on a simple pattern. In the example for ID, the token must start with a character from the alphas container and the rest of it can be comprised of either alphas, digits or underscore.

**#construct** joins multiple tokens together based on some simple rules. Eg.

```#construct STRING = DOUBLE_QUOTE - * - DOUBLE_QUOTE```

It finds a double quote token and collects all the tokens until it reaches another double quote. If we're capturing spaces, which I am in this scenario, then it constructs a string from those tokens. It adds a string token and deletes the rest.

Notice the "-" on either side of the star (the star denoting any token). This means that we don't want to include the double quotes. If it were this:

```#construct STRING = DOUBLE_QUOTE + * + DOUBLE_QUOTE```

This would include the double quotes.

**#transform** replaces the token name with a different one based on a rule. In the example above, if a NUM token contains 1 character from the container period (so essentially, 1 period as the container is simply ".") then it transforms it into a FLOAT.

Transforms currently have these operations:

**in:** if a token name is in a vector container.

**contains:** if a token name contains characters from a string container. You can use * instead of a number to mean "if it contains any number of characters from this container". Adding the "+" modifier to the number provided means "or more".

**excludes:** if a token name does not include any character from a given container.
