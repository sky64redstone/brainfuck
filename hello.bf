Brainfuck ignores all characters that aren't used in the syntax.

++++++++++           Set cell 0 to 10 (loop counter)
[                    Loop 10 times:
  >+++++++           Add 7 to cell 1
  >++++++++++        Add 10 to cell 2
  >+++               Add 3 to cell 3
  >+                 Add 1 to cell 4
  <<<<-              Go back to cell 0; subtract 1
]
                     After the loop:
                     Cell 0 = 0
                     Cell 1 = 70
                     Cell 2 = 100
                     Cell 3 = 30
                     Cell 4 = 10

>++.                 Cell 1: 70 plus 2 = 72       (H)
>+.                  Cell 2: 100 plus 1 = 101     (e)
+++++++..            Cell 2: 101 plus 7 = 108     (l; printed twice)
+++.                 Cell 2: 108 plus 3 = 111     (o)
>++.                 Cell 3: 30 plus 2 = 32       (space)
<<+++++++++++++++.   Cell 1: 72 plus 15 = 87      (W)
>.                   Cell 2: 111                  (o)
+++.                 Cell 2: 111 minus 3 = 114    (r)
------.              Cell 2: 114 minus 6 = 108    (l)
--------.            Cell 2: 108 minus 8 = 100    (d)
>+.                  Cell 3: 32 plus 1 = 33       (!)
>.                   Cell 4: 10                   (newline)
