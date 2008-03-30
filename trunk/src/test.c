#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "json.h"
#include "json_helper.h"

enum json_error json_tree_to_file (json_t * root, FILE *file)
{
}



#define BUFFER_SIZE 1024

int
main (void)
{
	char buffer[BUFFER_SIZE];
	char *temp = NULL;
	unsigned int error = JSON_INCOMPLETE_DOCUMENT;

	struct json_parsing_info state;

	json_jpi_init(&state);

	while  ( (error == JSON_WAITING_FOR_EOF) || (error == JSON_INCOMPLETE_DOCUMENT) )
	{
		if(fgets (buffer, BUFFER_SIZE, stdin) != NULL)
		{
			switch(error = json_parse_fragment ( &state, buffer))
			{
				case JSON_OK:
					printf("complete\n");
					json_tree_to_string(state.cursor, &temp);
					printf("%s\n",temp);
					break;

				case JSON_WAITING_FOR_EOF:
				case JSON_INCOMPLETE_DOCUMENT:
					break;

				default:
					printf("Some error occurred: %d\n", error);
			}
		}
		else
		{
			if(error == JSON_WAITING_FOR_EOF)
				error = JSON_OK;
			else
				error = JSON_UNKNOWN_PROBLEM;
		}
	}
	if(error == JSON_OK)
	{
		printf("printing...\n");
		json_tree_to_file(state.cursor, stdout);
	}
	else
	{
		printf("Document wasn't valid.\n");
	}
	/* perform cleanup */
	json_free_value(&state.cursor);
	return 0;
}
