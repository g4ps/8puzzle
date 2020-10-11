#!/usr/bin/awk -f
/shuffle/ {for (i = 0; i < 9; i++)
		printf("%d ", $(i + 8));
		printf("Depth: %d", $4);
		printf("\n");		
		/*printf("%s\n", $0);*/
		}

