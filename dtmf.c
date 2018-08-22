/* (c) 2018 Jan Stary <hans@stare.cz>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <err.h>

#define SRATE 8000

int dflag = 0;
int glen  = 50;
int tlen  = 100;

extern const char* __progname;

struct tone {
	unsigned char c;
	unsigned lo;
	unsigned hi;
} dtmf[] = {
	{ '1', 697, 1209 },
	{ '2', 697, 1336 },
	{ '3', 697, 1477 },
	{ 'A', 697, 1633 },
	{ '4', 770, 1209 },
	{ '5', 770, 1336 },
	{ '6', 770, 1477 },
	{ 'B', 770, 1633 },
	{ '7', 852, 1209 },
	{ '8', 852, 1336 },
	{ '9', 852, 1477 },
	{ 'C', 852, 1633 },
	{ '*', 941, 1209 },
	{ '0', 941, 1336 },
	{ '#', 941, 1477 },
	{ 'D', 941, 1633 },
	{   0,   0,    0 }
};

static void
usage()
{
	fprintf(stderr, "usage: %s [input] [output]\n", __progname);
}

/* Find the tone structure for the given char.
 * Return a pointer into the dtmf array, or NULL if not found. */
struct tone*
gettone(unsigned char c)
{
	struct tone *tone;
	for (tone = dtmf; tone && tone->c; tone++)
		if (tone->c == c)
			return tone;
	return NULL;
}

/* Write the audio data corresponding to char 'c'
 * into the open file descriptor fd.
 * Return what write(2) returns. */
ssize_t
wtone(int fd, unsigned char c)
{
	double t, a;
	unsigned int i;
	unsigned char *s;
	unsigned long T, G;
	struct tone *tone;
	if ((tone = gettone(c)) == NULL) {
		warnx("No tone found for '%c'", c);
		return -1;
	}
	T = (tlen / 1000.0) * SRATE;
	G = (glen / 1000.0) * SRATE;
	if ((s = calloc(T + G, 1)) == NULL)
		err(1, NULL);
	for (i = 0; i < T; i++) {
		t = (1.0 * i) / SRATE;
		a = sin(2 * M_PI * tone->lo * t) + sin(2 * M_PI * tone->hi * t);
		a *= M_SQRT2 / 4.0; /* normalize and attenuate */
		s[i] = UINT8_MAX * (0.5 * (a + 1.0));
		/*printf("%5u % -8.6f % -8.6f %3u\n", i, t, a, s[i]);*/
	}
	return write(fd, s, T + G);
}

int
chars2tones(int ifd, int ofd)
{
	ssize_t r, w;
	unsigned char c;
	while ((r = read(ifd, &c, 1)) > 0) {
		if (c == ' ' || c == '\t' || c == '\n')
			continue;
		if ((w = wtone(ofd, c)) == -1)
			break;
	}
	if (r == -1) {
		warnx("Error reading a symbol");
		return -1;
	}
	if (w == -1) {
		warnx("Error writing a tone for '%c'", c);
		return -1;
	}
	return 0;
}

int
tones2chars(int ifd, int ofd)
{
	return 0;
}

int
main(int argc, char** argv)
{
	int c;
	char *e = NULL;
	int ifd = STDIN_FILENO;
	int ofd = STDOUT_FILENO;

	while ((c = getopt(argc, argv, "dg:t:")) != -1) switch (c) {
		case 'd':
			dflag = 1;
			break;
		case 'g':
			glen = strtol(optarg, &e, 10);
			if (*e != '\0') {
				usage();
				return 1;
			}
			break;
		case 't':
			tlen = strtol(optarg, &e, 10);
			if (*e != '\0') {
				usage();
				return 1;
			}
			break;
		default:
			usage();
			return 1;
			break;
	}
	argc -= optind;
	argv += optind;

	if (glen <= 0 || tlen <= 0) {
		usage();
		return 1;
	}
	if (argc > 2) {
		usage();
		return 1;
	}
	if (argc > 0) {
		if (strcmp(*argv, "-")
		&& ((ifd = open(*argv, O_RDONLY)) == -1))
			err(1, "'%s'", *argv);
	}
	if (argc > 1) {
		if (strcmp(*++argv, "-")
		&& ((ofd = open(*argv, O_WRONLY|O_CREAT|O_TRUNC, 0640)) == -1))
			err(1, "'%s'", *argv);
	}
	return dflag
		? tones2chars(ifd, ofd)
		: chars2tones(ifd, ofd);
}
