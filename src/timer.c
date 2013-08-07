#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "playsound.h"

#define LCASE_LETTERS_LOWER_BOUND 'a'
#define LCASE_LETTERS_UPPER_BOUND 'z'
#define UCASE_LETTERS_LOWER_BOUND 'A'
#define UCASE_LETTERS_UPPER_BOUND 'Z'
#define NUMBERS_LOWER_BOUND '0'
#define NUMBERS_UPPER_BOUND '9'

#define IS_LOWERCASE_LETTER(x) (((x) >= LCASE_LETTERS_LOWER_BOUND && (x) <= LCASE_LETTERS_UPPER_BOUND))
#define IS_UPPERCASE_LETTER(x) (((x) >= UCASE_LETTERS_LOWER_BOUND && (x) <= UCASE_LETTERS_UPPER_BOUND)) 
#define IS_LETTER(x) 	       (IS_LOWERCASE_LETTER(x) || IS_UPPERCASE_LETTER(x))

#define IS_DIGIT(x) ((((x) >= NUMBERS_LOWER_BOUND && (x) <= NUMBERS_UPPER_BOUND)) || (x) == '.')
#define IS_DIGIT_OR_BRACE(x) ((IS_NUMBER(x) || (x == '(' || x == ')')))

#define CONTAINS_LETTERS 0x1
#define CONTAINS_NUMBERS 0x1 << 1
#define CONTAINS_SPECIAL_CHARACTERS 0x1 << 2
#define CONTAINS_BOTH (CONTAINS_LETTERS | CONTAINS_NUMBERS)

static void print_accepted_time_factors() {
	printf("Accepted time factor identifiers with their respective synonyms:\n s <=> sec <=> seconds\n m <=> min <=> minutes\n h <=> hours\n");
}


static void usage() {
	printf("Usage: timer TIMESTRING\n\n\
Valid TIMESTRING formatting examples:\n\
	1 h 35 min 2 sec\n\
	1h 35m 2s\n\
	1h 2 min 3m 3 hours (resolves to 4 hours 5 minutes)\n\
	35 (resolves to 35 seconds)\n\
	\n\
Individual time units must be separated by whitespace. Leaving them out results in undefined behavior.\n\n");
	print_accepted_time_factors();
}


typedef struct {
	char *str;
	size_t str_len;
	int arg_info;
} arg_unit;

char* substring(const char* string, size_t pos, size_t length) {

	char* ret = malloc(length+1);
	memcpy(ret, &string[pos], length);
	ret[length] = '\0';
	return ret;

}

static void unit_initialize(arg_unit *unit, char* arg) {

	unit->str = arg;
	unit->str_len = strlen(unit->str);
	int i = 0;
	unit->arg_info = 0;

	while (i < unit->str_len) {
		if (IS_LETTER(unit->str[i])) { unit->arg_info |= CONTAINS_LETTERS; }
		else if (IS_DIGIT(unit->str[i])) { unit->arg_info |= CONTAINS_NUMBERS; }
		else { unit->arg_info |= CONTAINS_SPECIAL_CHARACTERS; }
		++i;
	}
}

struct time_string_constant_pair {
	const char* time_string;
	int factor;
};

static const struct time_string_constant_pair time_table[] = {
	{ "h", 3600 },
	{ "hours", 3600 },
	{ "m", 60 },
	{ "min", 60 },
	{ "minutes", 60 },
	{ "s", 1 },
	{ "sec", 1 },
	{ "seconds", 1 }
};

static const size_t time_table_sz = sizeof(time_table)/sizeof(time_table[0]);

static int get_time_factor(const char *term) {
	int i = 0;
	while (i < time_table_sz) {
		if (strcmp(term, time_table[i].time_string) == 0) {
			return time_table[i].factor;
		}
		++i;
	}
	printf("error: unknown time factor \"%s\".\n", term);
	print_accepted_time_factors();
	return -1;
}

static int get_time_from_mixed(const char* arg, double *ret) {
	// the term should begin with only digits
	
	const size_t arg_len = strlen(arg);

	if (!IS_DIGIT(arg[0])) {
		printf("error: \"%s\": expected argument to begin with a digit.\n", arg);
		return -1;
	}

	size_t q_end_pos;

	size_t i = 0;
	while (IS_DIGIT(arg[i]) && i < arg_len) {
		++i;
	}
	q_end_pos = i;

	char *dummy;
	char *q_string = substring(arg, 0, q_end_pos);
	// the rest should be the string part, "sec" for instance
	char *str_part = substring(arg, q_end_pos, arg_len - q_end_pos);

	double q = strtod(q_string, &dummy);
	free(q_string);
	double factor = get_time_factor(str_part);
	free(str_part);

	*ret = factor == -1 ? -1 : factor*q;	

	return 1;

}

static long parse_cmdline_get_seconds(int argc, char *argv[]) {
	
	const size_t num_units = argc-1;
	arg_unit units[num_units];

	size_t i = 1;
	while (i < argc) {
		unit_initialize(&units[i-1], argv[i]);
		if (units[i-1].arg_info & CONTAINS_SPECIAL_CHARACTERS) { 
			printf("error: argument \"%s\" contains special characters.\n", units[i-1].str); 
			return -1;
		}
		++i;
	}

	if (num_units == 1) {
		if (units[0].arg_info == CONTAINS_NUMBERS) {
			char *dummy;
			return strtod(units[0].str, &dummy);
		}
	}
	i = 0;
	
	double t_sum_seconds = 0;
	while (i < num_units) {
		if (units[i].arg_info == CONTAINS_LETTERS) {
			if (i > 0) {
				if (units[i-1].arg_info != CONTAINS_NUMBERS) {
					printf("error: invalid unit \"%s\".\n", units[i].str);
					return -1;
				}
				else {
					// do a strcmp round
					double factor = get_time_factor(units[i].str);
					if (factor == -1) {
						return -1;
					}

					char *dummy;
					double time_arg = strtod(units[i-1].str, &dummy);
					t_sum_seconds += time_arg * factor;

				}
			}
			else {
				printf("error: invalid unit \"%s\".\n", units[i].str);
				return -1;
			}
		}
		else if (units[i].arg_info == CONTAINS_BOTH) {
			double t = 0;
		       	int ret = get_time_from_mixed(units[i].str, &t);
			if (ret < 0) {
				return -1;
			}
			t_sum_seconds += t;
		}
		++i;
	}

	return t_sum_seconds;

}

static void countdown(long seconds) {

	const char *escapes_joined = "\033[1K\r";
	unsigned long i = seconds;

	const long measured_overhead_adjustment = 170L * 1000L;	
	const long sleeptime = 1000000000L - measured_overhead_adjustment;
	const struct timespec t = { 0, sleeptime };

	while (i > 0) {
		
		unsigned long minutes = i/60;
		unsigned long seconds = i%60;

		// use stderr for unbuffered output :P 
		fprintf(stderr, "%s%lu min, %lu sec left", escapes_joined, minutes, seconds);
		nanosleep(&t, NULL);
		--i;
	}

	fprintf(stderr, "%s0 min, 0 sec left - ALARM!", escapes_joined);

}

extern char _binary_alarm_wav_start;
extern char _binary_alarm_wav_end;

int main(int argc, char *argv[]) {

	if (argc < 2) { usage(); return 1; }

	const long t = parse_cmdline_get_seconds(argc, argv);
	if (t == -1) { printf("error: invalid TIMESTRING - invoke this program without arguments for further information.\n"); return 1; }
	
	const size_t alarm_wav_size = &_binary_alarm_wav_end - &_binary_alarm_wav_start;
	createALContext();
	load_raw(&_binary_alarm_wav_start, alarm_wav_size);

	countdown(t);

	system("amixer sset Master 90% > /dev/null 2>&1");	// this is crap, but works alright for me :P
	playSound();

	while (is_still_playing());
	
	al_cleanup();
	system("amixer sset Master 30% > /dev/null 2>&1");	

	putchar('\n');
	return 0;
}
