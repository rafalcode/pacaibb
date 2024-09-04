#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

#define MAX_SKILLS 10

typedef struct
{
    char *name;
    int age;
    char *occupation;
    char *skills[MAX_SKILLS];
    int skill_count;
} Person;

void initialize_person(Person *person)
{
    person->name = NULL;
    person->age = 0;
    person->occupation = NULL;
    person->skill_count = 0;
    for (int i = 0; i < MAX_SKILLS; i++) {
        person->skills[i] = NULL;
    }
}

void free_person(Person *person) {
    if (person->name)
        free(person->name);
    if (person->occupation)
        free(person->occupation);
    for (int i = 0; i < person->skill_count; i++) {
        if (person->skills[i]) free(person->skills[i]);
    }
}

void parse_yaml_file(const char *filename, Person *person)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, "Failed to initialize parser!\n");
        fclose(file);
        return;
    }

    yaml_parser_set_input_file(&parser, file);

    char *key = NULL;
    int in_sequence = 0;

    while (1) {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "Parser error %d\n", parser.error);
            break;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            char *value = (char *)event.data.scalar.value;

            if (key == NULL) {
                key = strdup(value);  // Save the key
            } else {
                if (strcmp(key, "name") == 0) {
                    person->name = strdup(value);
                } else if (strcmp(key, "age") == 0) {
                    person->age = atoi(value);
                } else if (strcmp(key, "occupation") == 0) {
                    person->occupation = strdup(value);
                } else if (strcmp(key, "skills") == 0 || in_sequence) {
                    person->skills[person->skill_count] = strdup(value);
                    person->skill_count++;
                }
                free(key);  // Free the key after use
                key = NULL;
            }
        } else if (event.type == YAML_SEQUENCE_START_EVENT) {
            in_sequence = 1;
        } else if (event.type == YAML_SEQUENCE_END_EVENT) {
            in_sequence = 0;
        } else if (event.type == YAML_STREAM_END_EVENT) {
            break;
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(file);
}

void print_person(const Person *person)
{
    printf("Name: %s\n", person->name);
    printf("Age: %d\n", person->age);
    printf("Occupation: %s\n", person->occupation);
    printf("Skills:\n");
    for (int i = 0; i < person->skill_count; i++) {
        printf("  - %s\n", person->skills[i]);
    }
}

int main(void)
{
    Person person;
    initialize_person(&person);

    parse_yaml_file("johndoe.yaml", &person);

    print_person(&person);

    free_person(&person);

    return 0;
}
