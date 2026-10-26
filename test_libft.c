/* ************************************************************************** */
/*                                                                            */
/*   test_libft.c : testeur complet de la libft (sujet v19.2)                 */
/*                                                                            */
/*   /!\ NE PAS RENDRE CE FICHIER DANS LE DEPOT : le sujet interdit les       */
/*       fichiers inutiles. Il est seulement la pour la correction.           */
/*                                                                            */
/*   Utilisation (depuis la racine du depot, apres "make") :                  */
/*       cc -Wall -Wextra -Werror test_libft.c -L. -lft -o test_libft         */
/*       ./test_libft                                                         */
/*                                                                            */
/*   Fuites memoire (Linux) :                                                 */
/*       valgrind --leak-check=full ./test_libft                              */
/*                                                                            */
/*   Chaque fonction est comparee a la vraie fonction de la libc quand elle   */
/*   existe, sinon a des valeurs attendues ecrites a la main. Un segfault,    */
/*   un double free ou une boucle infinie dans une fonction est affiche       */
/*   comme [CRASH] et le testeur continue avec la fonction suivante.          */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <ctype.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GRN "\033[1;32m"
#define RED "\033[1;31m"
#define YEL "\033[1;33m"
#define CYA "\033[1;36m"
#define RST "\033[0m"

/* ============================== mini framework ============================ */

static sigjmp_buf				g_jmp;
static volatile sig_atomic_t	g_sig;
static void						(*g_test)(void);
static int						g_ok;
static int						g_ko;
static int						g_fn_ok;
static int						g_fn_total;
static char						g_log[16384];
static size_t					g_log_len;

static void	fail(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

static void	fail(const char *fmt, ...)
{
	va_list	ap;
	size_t	room;
	int		n;

	g_ko++;
	room = sizeof(g_log) - g_log_len;
	if (room < 32)
		return ;
	memcpy(g_log + g_log_len, "      -> ", 9);
	g_log_len += 9;
	room -= 9;
	va_start(ap, fmt);
	n = vsnprintf(g_log + g_log_len, room - 1, fmt, ap);
	va_end(ap);
	if (n < 0)
		n = 0;
	if ((size_t)n > room - 2)
		n = (int)(room - 2);
	g_log_len += (size_t)n;
	g_log[g_log_len++] = '\n';
	g_log[g_log_len] = '\0';
}

/* Les arguments du message ne sont evalues que si le test echoue. */
#define CHECK(cond, ...) \
	do { if (cond) g_ok++; else fail(__VA_ARGS__); } while (0)

/* Affiche une chaine entre guillemets avec les caracteres speciaux echappes. */
static const char	*esc(const char *s)
{
	static char	bufs[8][300];
	static int	idx;
	char		*b;
	size_t		i;

	if (!s)
		return ("NULL");
	b = bufs[idx++ % 8];
	i = 0;
	b[i++] = '"';
	while (*s && i < 280)
	{
		if (*s == '\n')
			i += (size_t)snprintf(b + i, 300 - i, "\\n");
		else if (*s == '\t')
			i += (size_t)snprintf(b + i, 300 - i, "\\t");
		else if (isprint((unsigned char)*s))
			b[i++] = *s;
		else
			i += (size_t)snprintf(b + i, 300 - i, "\\%03o", (unsigned char)*s);
		s++;
	}
	b[i++] = '"';
	b[i] = '\0';
	return (b);
}

/* Affiche un caractere lisible a cote de son code : " 'a'" ou "". */
static const char	*chr(int c)
{
	static char	bufs[4][8];
	static int	idx;
	char		*b;

	b = bufs[idx++ % 4];
	b[0] = '\0';
	if (c >= 0 && c <= 255 && isprint(c))
		snprintf(b, 8, " '%c'", c);
	return (b);
}

/* Affiche la position d'un pointeur dans une chaine : "s + 3" ou "NULL". */
static const char	*pos(const void *base, const void *p)
{
	static char	bufs[4][32];
	static int	idx;
	char		*b;

	if (!p)
		return ("NULL");
	b = bufs[idx++ % 4];
	snprintf(b, 32, "s + %ld", (long)((const char *)p - (const char *)base));
	return (b);
}

static int	sgn(int x)
{
	return ((x > 0) - (x < 0));
}

/* Compare une chaine allouee au resultat attendu, puis la libere. */
static void	check_str(char *got, const char *exp, const char *call)
{
	if (got && exp && strcmp(got, exp) == 0)
		g_ok++;
	else
		fail("%s : attendu %s, obtenu %s", call, esc(exp), esc(got));
	free(got);
}

static const char	*signame(int sig)
{
	if (sig == SIGSEGV)
		return ("SIGSEGV (segfault)");
	if (sig == SIGBUS)
		return ("SIGBUS (bus error)");
	if (sig == SIGABRT)
		return ("SIGABRT (double free / abort)");
	if (sig == SIGALRM)
		return ("TIMEOUT (boucle infinie ?)");
	return ("signal inattendu");
}

static void	on_signal(int sig)
{
	g_sig = sig;
	siglongjmp(g_jmp, 1);
}

static void	setup_signals(void)
{
	static char			altstack[1 << 16];
	stack_t				ss;
	struct sigaction	sa;

	ss.ss_sp = altstack;
	ss.ss_size = sizeof(altstack);
	ss.ss_flags = 0;
	sigaltstack(&ss, NULL);
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sa.sa_flags = SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
}

static int	protected_call(void)
{
	if (sigsetjmp(g_jmp, 1) == 0)
	{
		alarm(5);
		g_test();
		alarm(0);
		return (0);
	}
	alarm(0);
	return (1);
}

static void	run(const char *name, void (*test)(void))
{
	int	ok_before;
	int	ko_before;

	ok_before = g_ok;
	ko_before = g_ko;
	g_log_len = 0;
	g_log[0] = '\0';
	g_sig = 0;
	g_test = test;
	g_fn_total++;
	if (protected_call())
	{
		g_ko++;
		printf(RED "[CRASH]" RST " %-16s %s\n", name, signame(g_sig));
	}
	else if (g_ko == ko_before)
	{
		g_fn_ok++;
		printf(GRN "[OK]   " RST " %-16s %d/%d\n", name,
			g_ok - ok_before, g_ok - ok_before);
	}
	else
		printf(RED "[KO]   " RST " %-16s %d/%d\n", name,
			g_ok - ok_before, g_ok - ok_before + g_ko - ko_before);
	printf("%s", g_log);
}

static void	section(const char *title)
{
	printf("\n" CYA "==== %s ====" RST "\n", title);
}

/* Capture ce qu'une fonction ft_put*_fd ecrit, grace a un pipe. */
static void	capture_open(int p[2])
{
	if (pipe(p) < 0)
	{
		perror("pipe");
		exit(1);
	}
}

static const char	*capture_close(int p[2])
{
	static char	buf[512];
	size_t		len;
	ssize_t		n;

	close(p[1]);
	len = 0;
	while (len < sizeof(buf) - 1)
	{
		n = read(p[0], buf + len, sizeof(buf) - 1 - len);
		if (n <= 0)
			break ;
		len += (size_t)n;
	}
	buf[len] = '\0';
	close(p[0]);
	return (buf);
}

/* ===================== Partie 1 : fonctions de la libc ==================== */

/* Teste les 257 valeurs de -1 (EOF) a 255 contre la vraie fonction. */
static void	class_test(const char *name, int (*ft)(int), int (*real)(int))
{
	int	c;
	int	bad_true;
	int	bad_false;
	int	bad_ret;

	bad_true = INT_MIN;
	bad_false = INT_MIN;
	bad_ret = INT_MIN;
	c = -1;
	while (c <= 255)
	{
		if (bad_true == INT_MIN && real(c) && !ft(c))
			bad_true = c;
		if (bad_false == INT_MIN && !real(c) && ft(c))
			bad_false = c;
		if (bad_ret == INT_MIN && ft(c) != 0 && ft(c) != 1)
			bad_ret = c;
		c++;
	}
	CHECK(bad_true == INT_MIN, "%s(%d%s) : attendu vrai (1), obtenu %d",
		name, bad_true, chr(bad_true), ft(bad_true));
	CHECK(bad_false == INT_MIN, "%s(%d%s) : attendu faux (0), obtenu %d",
		name, bad_false, chr(bad_false), ft(bad_false));
	CHECK(bad_ret == INT_MIN,
		"%s(%d%s) renvoie %d : le sujet exige exactement 1 ou 0", name,
		bad_ret, chr(bad_ret), ft(bad_ret));
}

static void	t_isalpha(void)
{
	class_test("ft_isalpha", ft_isalpha, isalpha);
}

static void	t_isdigit(void)
{
	class_test("ft_isdigit", ft_isdigit, isdigit);
}

static void	t_isalnum(void)
{
	class_test("ft_isalnum", ft_isalnum, isalnum);
}

static void	t_isascii(void)
{
	class_test("ft_isascii", ft_isascii, isascii);
}

static void	t_isprint(void)
{
	class_test("ft_isprint", ft_isprint, isprint);
}

static void	t_strlen(void)
{
	char	big[1001];
	size_t	r;

	memset(big, 'x', 1000);
	big[1000] = '\0';
	r = ft_strlen("");
	CHECK(r == 0, "ft_strlen(\"\") : attendu 0, obtenu %zu", r);
	r = ft_strlen("a");
	CHECK(r == 1, "ft_strlen(\"a\") : attendu 1, obtenu %zu", r);
	r = ft_strlen("Ecole 42");
	CHECK(r == 8, "ft_strlen(\"Ecole 42\") : attendu 8, obtenu %zu", r);
	r = ft_strlen("ab\0cd");
	CHECK(r == 2, "ft_strlen(\"ab\\0cd\") : attendu 2, obtenu %zu", r);
	r = ft_strlen(big);
	CHECK(r == 1000, "ft_strlen(1000 x 'x') : attendu 1000, obtenu %zu", r);
}

static void	t_memset(void)
{
	char	a[20];
	char	b[20];
	void	*r;

	memset(a, 'x', 20);
	memset(b, 'x', 20);
	r = ft_memset(a, 'A', 10);
	memset(b, 'A', 10);
	CHECK(memcmp(a, b, 20) == 0,
		"ft_memset(buf, 'A', 10) : resultat different de memset");
	CHECK(r == a, "ft_memset doit renvoyer le pointeur de destination");
	ft_memset(a, 300, 5);
	memset(b, 300, 5);
	CHECK(memcmp(a, b, 20) == 0,
		"ft_memset(buf, 300, 5) : c doit etre converti en unsigned char");
	ft_memset(a + 15, 'Z', 0);
	CHECK(memcmp(a, b, 20) == 0, "ft_memset(buf, 'Z', 0) ne doit rien ecrire");
}

static void	t_bzero(void)
{
	char	a[10];
	int		i;
	int		ok;

	memset(a, 'x', 10);
	ft_bzero(a, 5);
	ok = 1;
	i = 0;
	while (i < 10)
	{
		if ((i < 5 && a[i] != 0) || (i >= 5 && a[i] != 'x'))
			ok = 0;
		i++;
	}
	CHECK(ok, "ft_bzero(buf, 5) : 5 premiers octets a 0, les autres intacts");
	ft_bzero(a + 5, 0);
	CHECK(a[5] == 'x', "ft_bzero(buf, 0) ne doit rien ecrire");
}

static void	t_memcpy(void)
{
	char	src[] = "ab\0cdef";
	char	dst[10];
	int		is[5];
	int		id[5];
	void	*r;

	memset(dst, 'x', 10);
	r = ft_memcpy(dst, src, 7);
	CHECK(memcmp(dst, src, 7) == 0,
		"ft_memcpy doit copier n octets, meme apres un '\\0'");
	CHECK(r == dst, "ft_memcpy doit renvoyer le pointeur de destination");
	CHECK(dst[7] == 'x', "ft_memcpy ne doit pas ecrire plus de n octets");
	ft_memcpy(dst, "zz", 0);
	CHECK(dst[0] == 'a', "ft_memcpy(dst, src, 0) ne doit rien copier");
	is[0] = 1;
	is[1] = -2;
	is[2] = 3;
	is[3] = INT_MAX;
	is[4] = INT_MIN;
	ft_memcpy(id, is, sizeof(is));
	CHECK(memcmp(id, is, sizeof(is)) == 0,
		"ft_memcpy sur un tableau d'int : copie incorrecte");
}

static void	t_memmove(void)
{
	char	buf[20];
	char	ref[20];
	void	*r;

	strcpy(buf, "0123456789");
	strcpy(ref, "0123456789");
	r = ft_memmove(buf + 2, buf, 5);
	memmove(ref + 2, ref, 5);
	CHECK(strcmp(buf, ref) == 0,
		"chevauchement (dst > src) : attendu %s, obtenu %s", esc(ref), esc(buf));
	CHECK(r == buf + 2, "ft_memmove doit renvoyer le pointeur de destination");
	strcpy(buf, "0123456789");
	strcpy(ref, "0123456789");
	ft_memmove(buf, buf + 3, 5);
	memmove(ref, ref + 3, 5);
	CHECK(strcmp(buf, ref) == 0,
		"chevauchement (dst < src) : attendu %s, obtenu %s", esc(ref), esc(buf));
	ft_memmove(buf, buf, 5);
	CHECK(strcmp(buf, ref) == 0, "ft_memmove(buf, buf, 5) ne doit rien changer");
	ft_memmove(buf, "zz", 0);
	CHECK(strcmp(buf, ref) == 0, "ft_memmove(dst, src, 0) ne doit rien copier");
}

static void	t_strlcpy(void)
{
	char	dst[16];
	size_t	r;

	memset(dst, 'x', 15);
	dst[15] = '\0';
	r = ft_strlcpy(dst, "hello", 16);
	CHECK(r == 5 && strcmp(dst, "hello") == 0, "ft_strlcpy(dst, \"hello\", 16)"
		" : attendu 5 et \"hello\", obtenu %zu et %s", r, esc(dst));
	r = ft_strlcpy(dst, "hello", 3);
	CHECK(r == 5 && strcmp(dst, "he") == 0, "ft_strlcpy(dst, \"hello\", 3)"
		" : attendu 5 et \"he\", obtenu %zu et %s", r, esc(dst));
	r = ft_strlcpy(dst, "hello", 1);
	CHECK(r == 5 && dst[0] == '\0', "ft_strlcpy(dst, \"hello\", 1)"
		" : attendu 5 et \"\", obtenu %zu et %s", r, esc(dst));
	strcpy(dst, "abc");
	r = ft_strlcpy(dst, "hello", 0);
	CHECK(r == 5 && strcmp(dst, "abc") == 0, "ft_strlcpy(dst, \"hello\", 0)"
		" : attendu 5 et dst intact, obtenu %zu et %s", r, esc(dst));
	r = ft_strlcpy(dst, "", 16);
	CHECK(r == 0 && dst[0] == '\0', "ft_strlcpy(dst, \"\", 16)"
		" : attendu 0 et \"\", obtenu %zu et %s", r, esc(dst));
	r = ft_strlcpy(dst, "hello", 6);
	CHECK(r == 5 && strcmp(dst, "hello") == 0, "ft_strlcpy(dst, \"hello\", 6)"
		" : attendu 5 et \"hello\", obtenu %zu et %s", r, esc(dst));
}

static void	t_strlcat(void)
{
	char	dst[20];
	size_t	r;

	memset(dst, 'x', 19);
	dst[19] = '\0';
	strcpy(dst, "abc");
	r = ft_strlcat(dst, "def", 20);
	CHECK(r == 6 && strcmp(dst, "abcdef") == 0, "ft_strlcat(\"abc\", \"def\", 20)"
		" : attendu 6 et \"abcdef\", obtenu %zu et %s", r, esc(dst));
	strcpy(dst, "abc");
	r = ft_strlcat(dst, "def", 5);
	CHECK(r == 6 && strcmp(dst, "abcd") == 0, "ft_strlcat(\"abc\", \"def\", 5)"
		" : attendu 6 et \"abcd\", obtenu %zu et %s", r, esc(dst));
	strcpy(dst, "abc");
	r = ft_strlcat(dst, "def", 3);
	CHECK(r == 6 && strcmp(dst, "abc") == 0, "ft_strlcat(\"abc\", \"def\", 3)"
		" : attendu 6 et \"abc\", obtenu %zu et %s", r, esc(dst));
	strcpy(dst, "abc");
	r = ft_strlcat(dst, "def", 2);
	CHECK(r == 5 && strcmp(dst, "abc") == 0, "ft_strlcat(\"abc\", \"def\", 2)"
		" : attendu 5 et \"abc\", obtenu %zu et %s", r, esc(dst));
	strcpy(dst, "abc");
	r = ft_strlcat(dst, "def", 0);
	CHECK(r == 3 && strcmp(dst, "abc") == 0, "ft_strlcat(\"abc\", \"def\", 0)"
		" : attendu 3 et \"abc\", obtenu %zu et %s", r, esc(dst));
	dst[0] = '\0';
	r = ft_strlcat(dst, "hello", 3);
	CHECK(r == 5 && strcmp(dst, "he") == 0, "ft_strlcat(\"\", \"hello\", 3)"
		" : attendu 5 et \"he\", obtenu %zu et %s", r, esc(dst));
	strcpy(dst, "abc");
	r = ft_strlcat(dst, "", 20);
	CHECK(r == 3 && strcmp(dst, "abc") == 0, "ft_strlcat(\"abc\", \"\", 20)"
		" : attendu 3 et \"abc\", obtenu %zu et %s", r, esc(dst));
}

static void	case_test(const char *name, int (*ft)(int), int (*real)(int))
{
	int	c;
	int	bad;

	bad = INT_MIN;
	c = -1;
	while (c <= 255)
	{
		if (bad == INT_MIN && ft(c) != real(c))
			bad = c;
		c++;
	}
	CHECK(bad == INT_MIN, "%s(%d%s) : attendu %d, obtenu %d", name, bad,
		chr(bad), real(bad), ft(bad));
}

static void	t_toupper(void)
{
	case_test("ft_toupper", ft_toupper, toupper);
}

static void	t_tolower(void)
{
	case_test("ft_tolower", ft_tolower, tolower);
}

static void	t_strchr(void)
{
	const char	*s = "bonjour";
	const int	cs[] = {'b', 'o', 'r', 'z', '\0', 'j' + 256};
	size_t		i;
	char		*got;
	char		*exp;

	i = 0;
	while (i < sizeof(cs) / sizeof(cs[0]))
	{
		got = ft_strchr(s, cs[i]);
		exp = strchr(s, cs[i]);
		CHECK(got == exp, "ft_strchr(\"bonjour\", %d%s) : attendu %s, obtenu %s",
			cs[i], chr(cs[i]), pos(s, exp), pos(s, got));
		i++;
	}
	got = ft_strchr("", '\0');
	CHECK(got != NULL && *got == '\0',
		"ft_strchr(\"\", '\\0') doit renvoyer un pointeur sur le '\\0'");
}

static void	t_strrchr(void)
{
	const char	*s = "bonjour";
	const int	cs[] = {'b', 'o', 'r', 'z', '\0', 'o' + 256};
	size_t		i;
	char		*got;
	char		*exp;

	i = 0;
	while (i < sizeof(cs) / sizeof(cs[0]))
	{
		got = ft_strrchr(s, cs[i]);
		exp = strrchr(s, cs[i]);
		CHECK(got == exp, "ft_strrchr(\"bonjour\", %d%s) : attendu %s, obtenu %s",
			cs[i], chr(cs[i]), pos(s, exp), pos(s, got));
		i++;
	}
	got = ft_strrchr("", '\0');
	CHECK(got != NULL && *got == '\0',
		"ft_strrchr(\"\", '\\0') doit renvoyer un pointeur sur le '\\0'");
}

typedef struct s_cmp
{
	const char	*a;
	const char	*b;
	size_t		n;
}	t_cmp;

static void	t_strncmp(void)
{
	const t_cmp	cases[] = {
	{"abc", "abc", 3}, {"abc", "abd", 3}, {"abd", "abc", 3},
	{"abc", "abd", 2}, {"abc", "ab", 3}, {"ab", "abc", 3},
	{"", "a", 1}, {"a", "", 1}, {"abc", "abc", 100}, {"abc", "xyz", 0},
	{"\200", "\0", 1}, {"test\200", "test\0", 6}, {"\377", "a", 1}};
	size_t		i;
	int			got;
	int			exp;

	i = 0;
	while (i < sizeof(cases) / sizeof(cases[0]))
	{
		got = ft_strncmp(cases[i].a, cases[i].b, cases[i].n);
		exp = strncmp(cases[i].a, cases[i].b, cases[i].n);
		CHECK(sgn(got) == sgn(exp), "ft_strncmp(%s, %s, %zu) : attendu un "
			"resultat de signe %d, obtenu %d", esc(cases[i].a),
			esc(cases[i].b), cases[i].n, sgn(exp), got);
		i++;
	}
}

static void	t_memchr(void)
{
	const char	s[] = "bonjour\0x\377";
	void		*got;

	got = ft_memchr(s, 'j', 7);
	CHECK(got == s + 3, "ft_memchr(\"bonjour\", 'j', 7) : attendu s + 3, "
		"obtenu %s", pos(s, got));
	got = ft_memchr(s, 'z', 7);
	CHECK(got == NULL, "ft_memchr(\"bonjour\", 'z', 7) : attendu NULL, "
		"obtenu %s", pos(s, got));
	got = ft_memchr(s, 'r', 3);
	CHECK(got == NULL, "ft_memchr(\"bonjour\", 'r', 3) : attendu NULL "
		"(ne doit pas lire plus de n octets), obtenu %s", pos(s, got));
	got = ft_memchr(s, 'x', 10);
	CHECK(got == s + 8, "ft_memchr doit continuer apres un '\\0' : attendu "
		"s + 8, obtenu %s", pos(s, got));
	got = ft_memchr(s, -1, 11);
	CHECK(got == s + 9, "ft_memchr(s, -1, n) doit trouver l'octet 0xFF "
		"(c converti en unsigned char) : attendu s + 9, obtenu %s",
		pos(s, got));
	got = ft_memchr(s, 'b' + 256, 7);
	CHECK(got == s, "ft_memchr(s, 'b' + 256, 7) : attendu s + 0, obtenu %s",
		pos(s, got));
	got = ft_memchr(s, 'b', 0);
	CHECK(got == NULL, "ft_memchr(s, 'b', 0) : attendu NULL, obtenu %s",
		pos(s, got));
}

static void	t_memcmp(void)
{
	const t_cmp	cases[] = {
	{"abc", "abc", 3}, {"abc", "abd", 3}, {"abd", "abc", 3},
	{"abc", "abd", 2}, {"abc", "xyz", 0}, {"\200", "\0", 1},
	{"ab\0c", "ab\0d", 4}, {"ab\0d", "ab\0c", 4}, {"\377", "a", 1}};
	size_t		i;
	int			got;
	int			exp;

	i = 0;
	while (i < sizeof(cases) / sizeof(cases[0]))
	{
		got = ft_memcmp(cases[i].a, cases[i].b, cases[i].n);
		exp = memcmp(cases[i].a, cases[i].b, cases[i].n);
		CHECK(sgn(got) == sgn(exp), "ft_memcmp(%s, %s, %zu) : attendu un "
			"resultat de signe %d, obtenu %d", esc(cases[i].a),
			esc(cases[i].b), cases[i].n, sgn(exp), got);
		i++;
	}
}

static void	t_strnstr(void)
{
	const char	*big = "Foo Bar Baz";
	const char	*s = "aaabc";
	char		*got;

	got = ft_strnstr(big, "Bar", 11);
	CHECK(got == big + 4, "ft_strnstr(\"Foo Bar Baz\", \"Bar\", 11) : "
		"attendu s + 4, obtenu %s", pos(big, got));
	got = ft_strnstr(big, "Bar", 7);
	CHECK(got == big + 4, "ft_strnstr(\"Foo Bar Baz\", \"Bar\", 7) : "
		"attendu s + 4, obtenu %s", pos(big, got));
	got = ft_strnstr(big, "Bar", 6);
	CHECK(got == NULL, "ft_strnstr(\"Foo Bar Baz\", \"Bar\", 6) : "
		"attendu NULL, obtenu %s", pos(big, got));
	got = ft_strnstr(big, "", 11);
	CHECK(got == big, "ft_strnstr(big, \"\", 11) : attendu big (s + 0), "
		"obtenu %s", pos(big, got));
	got = ft_strnstr(big, "xyz", 11);
	CHECK(got == NULL, "ft_strnstr(big, \"xyz\", 11) : attendu NULL, "
		"obtenu %s", pos(big, got));
	got = ft_strnstr(s, "aabc", 5);
	CHECK(got == s + 1, "ft_strnstr(\"aaabc\", \"aabc\", 5) : attendu s + 1, "
		"obtenu %s", pos(s, got));
	got = ft_strnstr(big, "F", 0);
	CHECK(got == NULL, "ft_strnstr(big, \"F\", 0) : attendu NULL, obtenu %s",
		pos(big, got));
	got = ft_strnstr("abc", "abcd", 10);
	CHECK(got == NULL, "ft_strnstr(\"abc\", \"abcd\", 10) : attendu NULL, "
		"obtenu non NULL");
	got = ft_strnstr(big, "Baz", 100);
	CHECK(got == big + 8, "ft_strnstr(big, \"Baz\", 100) : attendu s + 8, "
		"obtenu %s", pos(big, got));
}

static void	t_atoi(void)
{
	const char	*cases[] = {"42", "   -42", "\t\n\v\f\r 42", "+42", "--42",
		"+-42", "-+42", "42abc", "abc42", "2147483647", "-2147483648", "0",
		"", "  +0012", "- 42", "1 2", "-0", "  \t-999x"};
	size_t		i;
	int			got;
	int			exp;

	i = 0;
	while (i < sizeof(cases) / sizeof(cases[0]))
	{
		got = ft_atoi(cases[i]);
		exp = atoi(cases[i]);
		CHECK(got == exp, "ft_atoi(%s) : attendu %d, obtenu %d",
			esc(cases[i]), exp, got);
		i++;
	}
}

static void	t_calloc(void)
{
	int		*p;
	void	*z1;
	void	*z2;
	int		i;
	int		ok;

	p = ft_calloc(10, sizeof(int));
	CHECK(p != NULL, "ft_calloc(10, sizeof(int)) a renvoye NULL");
	if (p)
	{
		ok = 1;
		i = 0;
		while (i < 10)
			if (p[i++] != 0)
				ok = 0;
		CHECK(ok, "ft_calloc(10, sizeof(int)) : la memoire doit etre a 0");
		free(p);
	}
	z1 = ft_calloc(0, 0);
	z2 = ft_calloc(0, 0);
	CHECK(z1 != NULL && z2 != NULL, "ft_calloc(0, 0) doit renvoyer un "
		"pointeur non NULL qu'on peut passer a free() (regle du sujet)");
	CHECK(z1 == NULL || z1 != z2, "ft_calloc(0, 0) doit renvoyer un "
		"pointeur unique a chaque appel");
	free(z1);
	free(z2);
	z1 = ft_calloc(0, 5);
	z2 = ft_calloc(5, 0);
	CHECK(z1 != NULL && z2 != NULL,
		"ft_calloc(0, 5) et ft_calloc(5, 0) ne doivent pas renvoyer NULL");
	free(z1);
	free(z2);
	z1 = ft_calloc(SIZE_MAX, 2);
	CHECK(z1 == NULL, "ft_calloc(SIZE_MAX, 2) : depassement, doit renvoyer "
		"NULL");
	free(z1);
}

static void	t_strdup(void)
{
	const char	*src = "Bonjour 42";
	char		*d;

	d = ft_strdup(src);
	CHECK(d != src, "ft_strdup doit renvoyer une nouvelle chaine, pas src");
	check_str(d, "Bonjour 42", "ft_strdup(\"Bonjour 42\")");
	check_str(ft_strdup(""), "", "ft_strdup(\"\")");
}

/* =================== Partie 2 : fonctions supplementaires ================= */

static void	t_substr(void)
{
	check_str(ft_substr("Bonjour", 0, 3), "Bon", "ft_substr(\"Bonjour\", 0, 3)");
	check_str(ft_substr("Bonjour", 3, 100), "jour",
		"ft_substr(\"Bonjour\", 3, 100)");
	check_str(ft_substr("Bonjour", 2, SIZE_MAX), "njour",
		"ft_substr(\"Bonjour\", 2, SIZE_MAX)");
	check_str(ft_substr("Bonjour", 7, 3), "", "ft_substr(\"Bonjour\", 7, 3)");
	check_str(ft_substr("Bonjour", 100, 3), "",
		"ft_substr(\"Bonjour\", 100, 3)");
	check_str(ft_substr("Bonjour", 2, 0), "", "ft_substr(\"Bonjour\", 2, 0)");
	check_str(ft_substr("", 0, 5), "", "ft_substr(\"\", 0, 5)");
}

static void	t_strjoin(void)
{
	check_str(ft_strjoin("Bonjour ", "42"), "Bonjour 42",
		"ft_strjoin(\"Bonjour \", \"42\")");
	check_str(ft_strjoin("", ""), "", "ft_strjoin(\"\", \"\")");
	check_str(ft_strjoin("abc", ""), "abc", "ft_strjoin(\"abc\", \"\")");
	check_str(ft_strjoin("", "abc"), "abc", "ft_strjoin(\"\", \"abc\")");
}

static void	t_strtrim(void)
{
	check_str(ft_strtrim("  xxBonjourxx  ", " x"), "Bonjour",
		"ft_strtrim(\"  xxBonjourxx  \", \" x\")");
	check_str(ft_strtrim("Bonjour", ""), "Bonjour",
		"ft_strtrim(\"Bonjour\", \"\")");
	check_str(ft_strtrim("xxxx", "x"), "", "ft_strtrim(\"xxxx\", \"x\")");
	check_str(ft_strtrim("", "abc"), "", "ft_strtrim(\"\", \"abc\")");
	check_str(ft_strtrim("abcHelloabc", "abc"), "Hello",
		"ft_strtrim(\"abcHelloabc\", \"abc\")");
	check_str(ft_strtrim("  a  ", " "), "a", "ft_strtrim(\"  a  \", \" \")");
	check_str(ft_strtrim(" a b ", " "), "a b", "ft_strtrim(\" a b \", \" \")");
}

static void	free_tab(char **tab)
{
	size_t	i;

	if (!tab)
		return ;
	i = 0;
	while (tab[i])
		free(tab[i++]);
	free(tab);
}

static void	check_split(const char *s, char c, const char **exp,
	const char *call)
{
	char	**res;
	size_t	i;

	res = ft_split(s, c);
	if (!res)
	{
		fail("%s : a renvoye NULL", call);
		return ;
	}
	i = 0;
	while (exp[i] && res[i] && strcmp(exp[i], res[i]) == 0)
		i++;
	if (exp[i] == NULL && res[i] == NULL)
		g_ok++;
	else
		fail("%s : case %zu, attendu %s, obtenu %s", call, i, esc(exp[i]),
			esc(res[i]));
	free_tab(res);
}

static void	t_split(void)
{
	const char	*e1[] = {"Bonjour", "le", "monde", NULL};
	const char	*e2[] = {NULL};
	const char	*e3[] = {"abc", NULL};
	const char	*e4[] = {"a", "b", NULL};
	const char	*e5[] = {"un", "deux", "trois", NULL};

	check_split("  Bonjour  le   monde ", ' ', e1,
		"ft_split(\"  Bonjour  le   monde \", ' ')");
	check_split("Bonjour le monde", ' ', e1,
		"ft_split(\"Bonjour le monde\", ' ')");
	check_split("", ' ', e2, "ft_split(\"\", ' ')");
	check_split("     ", ' ', e2, "ft_split(\"     \", ' ')");
	check_split("abc", ' ', e3, "ft_split(\"abc\", ' ')");
	check_split("abc", '\0', e3, "ft_split(\"abc\", '\\0')");
	check_split(",,a,,b,,", ',', e4, "ft_split(\",,a,,b,,\", ',')");
	check_split("un,deux,trois", ',', e5, "ft_split(\"un,deux,trois\", ',')");
}

static void	t_itoa(void)
{
	check_str(ft_itoa(0), "0", "ft_itoa(0)");
	check_str(ft_itoa(7), "7", "ft_itoa(7)");
	check_str(ft_itoa(-7), "-7", "ft_itoa(-7)");
	check_str(ft_itoa(42), "42", "ft_itoa(42)");
	check_str(ft_itoa(-42), "-42", "ft_itoa(-42)");
	check_str(ft_itoa(100), "100", "ft_itoa(100)");
	check_str(ft_itoa(INT_MAX), "2147483647", "ft_itoa(INT_MAX)");
	check_str(ft_itoa(INT_MIN), "-2147483648", "ft_itoa(INT_MIN)");
}

static char	map_even_upper(unsigned int i, char c)
{
	if (i % 2 == 0)
		return ((char)toupper((unsigned char)c));
	return (c);
}

static char	map_index(unsigned int i, char c)
{
	(void)c;
	return ((char)('0' + i % 10));
}

static void	t_strmapi(void)
{
	check_str(ft_strmapi("abcdef", map_even_upper), "AbCdEf",
		"ft_strmapi(\"abcdef\", majuscule si index pair)");
	check_str(ft_strmapi("xxxxx", map_index), "01234",
		"ft_strmapi(\"xxxxx\", renvoie l'index)");
	check_str(ft_strmapi("", map_index), "", "ft_strmapi(\"\", f)");
}

static int	g_iter_calls;

static void	iter_even_upper(unsigned int i, char *c)
{
	g_iter_calls++;
	if (i % 2 == 0)
		*c = (char)toupper((unsigned char)*c);
}

static void	iter_index(unsigned int i, char *c)
{
	*c = (char)('0' + i % 10);
}

static void	t_striteri(void)
{
	char	s1[] = "abcdef";
	char	s2[] = "xxxxx";
	char	s3[] = "";

	ft_striteri(s1, iter_even_upper);
	CHECK(strcmp(s1, "AbCdEf") == 0, "ft_striteri(\"abcdef\", majuscule si "
		"index pair) : attendu \"AbCdEf\", obtenu %s", esc(s1));
	ft_striteri(s2, iter_index);
	CHECK(strcmp(s2, "01234") == 0, "ft_striteri(\"xxxxx\", met l'index) : "
		"attendu \"01234\", obtenu %s", esc(s2));
	g_iter_calls = 0;
	ft_striteri(s3, iter_even_upper);
	CHECK(g_iter_calls == 0, "ft_striteri(\"\", f) ne doit jamais appeler f "
		"(appelee %d fois)", g_iter_calls);
}

static void	t_putchar_fd(void)
{
	int			p[2];
	const char	*out;

	capture_open(p);
	ft_putchar_fd('a', p[1]);
	ft_putchar_fd('\n', p[1]);
	ft_putchar_fd('Z', p[1]);
	out = capture_close(p);
	CHECK(strcmp(out, "a\nZ") == 0, "ft_putchar_fd 'a', '\\n', 'Z' : attendu "
		"\"a\\nZ\", obtenu %s", esc(out));
}

static void	t_putstr_fd(void)
{
	int			p[2];
	const char	*out;

	capture_open(p);
	ft_putstr_fd("Bonjour 42", p[1]);
	out = capture_close(p);
	CHECK(strcmp(out, "Bonjour 42") == 0, "ft_putstr_fd(\"Bonjour 42\") : "
		"obtenu %s", esc(out));
	capture_open(p);
	ft_putstr_fd("", p[1]);
	out = capture_close(p);
	CHECK(out[0] == '\0', "ft_putstr_fd(\"\") ne doit rien ecrire, obtenu %s",
		esc(out));
}

static void	t_putendl_fd(void)
{
	int			p[2];
	const char	*out;

	capture_open(p);
	ft_putendl_fd("Bonjour", p[1]);
	out = capture_close(p);
	CHECK(strcmp(out, "Bonjour\n") == 0, "ft_putendl_fd(\"Bonjour\") : "
		"attendu \"Bonjour\\n\", obtenu %s", esc(out));
	capture_open(p);
	ft_putendl_fd("", p[1]);
	out = capture_close(p);
	CHECK(strcmp(out, "\n") == 0, "ft_putendl_fd(\"\") : attendu \"\\n\", "
		"obtenu %s", esc(out));
}

static void	t_putnbr_fd(void)
{
	const int	nbs[] = {0, 7, -7, 42, -42, 100, INT_MAX, INT_MIN};
	char		exp[32];
	int			p[2];
	const char	*out;
	size_t		i;

	i = 0;
	while (i < sizeof(nbs) / sizeof(nbs[0]))
	{
		snprintf(exp, sizeof(exp), "%d", nbs[i]);
		capture_open(p);
		ft_putnbr_fd(nbs[i], p[1]);
		out = capture_close(p);
		CHECK(strcmp(out, exp) == 0, "ft_putnbr_fd(%d) : attendu \"%s\", "
			"obtenu %s", nbs[i], exp, esc(out));
		i++;
	}
}

/* ======================== Partie 3 : listes chainees ====================== */

static int	g_del_calls;

static int	*new_int(int v)
{
	int	*p;

	p = malloc(sizeof(int));
	if (!p)
	{
		perror("malloc");
		exit(1);
	}
	*p = v;
	return (p);
}

static void	del_count(void *p)
{
	g_del_calls++;
	free(p);
}

static void	inc_int(void *p)
{
	(*(int *)p)++;
}

static void	*double_int(void *p)
{
	return (new_int(*(int *)p * 2));
}

/* Ces helpers n'utilisent pas ta libft, pour tester chaque fonction seule. */
static t_list	*mk_node(int v)
{
	t_list	*n;

	n = malloc(sizeof(t_list));
	if (!n)
	{
		perror("malloc");
		exit(1);
	}
	n->content = new_int(v);
	n->next = NULL;
	return (n);
}

static t_list	*mk_list3(void)
{
	t_list	*l;

	l = mk_node(1);
	l->next = mk_node(2);
	l->next->next = mk_node(3);
	return (l);
}

static void	free_list(t_list *l)
{
	t_list	*next;

	while (l)
	{
		next = l->next;
		free(l->content);
		free(l);
		l = next;
	}
}

static void	t_lstnew(void)
{
	int		x;
	t_list	*n;

	x = 42;
	n = ft_lstnew(&x);
	CHECK(n != NULL, "ft_lstnew(&x) a renvoye NULL");
	if (n)
	{
		CHECK(n->content == &x, "content doit etre le pointeur donne");
		CHECK(n->next == NULL, "next doit etre initialise a NULL");
		free(n);
	}
	n = ft_lstnew(NULL);
	CHECK(n != NULL && n->content == NULL && n->next == NULL,
		"ft_lstnew(NULL) : content et next doivent valoir NULL");
	free(n);
}

static void	t_lstadd_front(void)
{
	t_list	*lst;
	t_list	*a;
	t_list	*b;

	lst = NULL;
	a = mk_node(1);
	b = mk_node(2);
	ft_lstadd_front(&lst, a);
	CHECK(lst == a && a->next == NULL,
		"ajout dans une liste vide : la liste doit pointer sur le nouvel element");
	ft_lstadd_front(&lst, b);
	CHECK(lst == b && b->next == a,
		"le nouvel element doit devenir la tete et pointer sur l'ancienne");
	free_list(lst);
}

static void	t_lstsize(void)
{
	t_list	*l;
	int		r;

	r = ft_lstsize(NULL);
	CHECK(r == 0, "ft_lstsize(NULL) : attendu 0, obtenu %d", r);
	l = mk_node(1);
	r = ft_lstsize(l);
	CHECK(r == 1, "ft_lstsize(1 element) : attendu 1, obtenu %d", r);
	free_list(l);
	l = mk_list3();
	r = ft_lstsize(l);
	CHECK(r == 3, "ft_lstsize(3 elements) : attendu 3, obtenu %d", r);
	free_list(l);
}

static void	t_lstlast(void)
{
	t_list	*l;

	CHECK(ft_lstlast(NULL) == NULL, "ft_lstlast(NULL) doit renvoyer NULL");
	l = mk_node(1);
	CHECK(ft_lstlast(l) == l, "ft_lstlast(1 element) doit renvoyer cet element");
	free_list(l);
	l = mk_list3();
	CHECK(ft_lstlast(l) == l->next->next,
		"ft_lstlast(3 elements) doit renvoyer le 3e element");
	free_list(l);
}

static void	t_lstadd_back(void)
{
	t_list	*lst;
	t_list	*n;

	lst = NULL;
	n = mk_node(1);
	ft_lstadd_back(&lst, n);
	CHECK(lst == n, "ajout dans une liste vide : la liste doit pointer sur "
		"le nouvel element");
	free_list(lst);
	lst = mk_list3();
	n = mk_node(4);
	ft_lstadd_back(&lst, n);
	CHECK(lst->next->next->next == n && n->next == NULL,
		"le nouvel element doit etre ajoute apres le dernier");
	free_list(lst);
}

static void	t_lstdelone(void)
{
	t_list	*a;
	t_list	*b;

	a = mk_node(1);
	b = mk_node(2);
	a->next = b;
	g_del_calls = 0;
	ft_lstdelone(a, del_count);
	CHECK(g_del_calls == 1, "del doit etre appelee 1 fois (appelee %d fois)",
		g_del_calls);
	CHECK(*(int *)b->content == 2, "l'element suivant ne doit pas etre "
		"libere");
	free_list(b);
}

static void	t_lstclear(void)
{
	t_list	*lst;

	lst = mk_list3();
	g_del_calls = 0;
	ft_lstclear(&lst, del_count);
	CHECK(g_del_calls == 3, "del doit etre appelee sur les 3 elements "
		"(appelee %d fois)", g_del_calls);
	CHECK(lst == NULL, "la liste doit etre mise a NULL a la fin");
	lst = NULL;
	ft_lstclear(&lst, del_count);
	CHECK(lst == NULL, "ft_lstclear sur une liste vide ne doit rien faire");
}

static void	t_lstiter(void)
{
	t_list	*lst;

	lst = mk_list3();
	ft_lstiter(lst, inc_int);
	CHECK(*(int *)lst->content == 2 && *(int *)lst->next->content == 3
		&& *(int *)lst->next->next->content == 4,
		"f doit etre appliquee au contenu de chaque element");
	free_list(lst);
	ft_lstiter(NULL, inc_int);
	g_ok++;
}

static void	t_lstmap(void)
{
	t_list	*lst;
	t_list	*map;

	lst = mk_list3();
	map = ft_lstmap(lst, double_int, free);
	CHECK(map != NULL, "ft_lstmap a renvoye NULL");
	if (map)
	{
		CHECK(map != lst && map->content != lst->content,
			"ft_lstmap doit creer une NOUVELLE liste");
		CHECK(map->next && map->next->next && !map->next->next->next
			&& *(int *)map->content == 2 && *(int *)map->next->content == 4
			&& *(int *)map->next->next->content == 6,
			"liste 1 -> 2 -> 3 avec f = x * 2 : attendu 2 -> 4 -> 6");
		CHECK(*(int *)lst->content == 1 && *(int *)lst->next->content == 2
			&& *(int *)lst->next->next->content == 3,
			"la liste d'origine ne doit pas etre modifiee");
	}
	free_list(map);
	free_list(lst);
	CHECK(ft_lstmap(NULL, double_int, free) == NULL,
		"ft_lstmap(NULL, f, del) doit renvoyer NULL");
}

/* ================================== main ================================== */

int	main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	setup_signals();
	printf(YEL "\n          TESTEUR LIBFT (sujet v19.2)\n" RST);
	section("Partie 1 - Fonctions de la libc");
	run("ft_isalpha", t_isalpha);
	run("ft_isdigit", t_isdigit);
	run("ft_isalnum", t_isalnum);
	run("ft_isascii", t_isascii);
	run("ft_isprint", t_isprint);
	run("ft_strlen", t_strlen);
	run("ft_memset", t_memset);
	run("ft_bzero", t_bzero);
	run("ft_memcpy", t_memcpy);
	run("ft_memmove", t_memmove);
	run("ft_strlcpy", t_strlcpy);
	run("ft_strlcat", t_strlcat);
	run("ft_toupper", t_toupper);
	run("ft_tolower", t_tolower);
	run("ft_strchr", t_strchr);
	run("ft_strrchr", t_strrchr);
	run("ft_strncmp", t_strncmp);
	run("ft_memchr", t_memchr);
	run("ft_memcmp", t_memcmp);
	run("ft_strnstr", t_strnstr);
	run("ft_atoi", t_atoi);
	run("ft_calloc", t_calloc);
	run("ft_strdup", t_strdup);
	section("Partie 2 - Fonctions supplementaires");
	run("ft_substr", t_substr);
	run("ft_strjoin", t_strjoin);
	run("ft_strtrim", t_strtrim);
	run("ft_split", t_split);
	run("ft_itoa", t_itoa);
	run("ft_strmapi", t_strmapi);
	run("ft_striteri", t_striteri);
	run("ft_putchar_fd", t_putchar_fd);
	run("ft_putstr_fd", t_putstr_fd);
	run("ft_putendl_fd", t_putendl_fd);
	run("ft_putnbr_fd", t_putnbr_fd);
	section("Partie 3 - Listes chainees");
	run("ft_lstnew", t_lstnew);
	run("ft_lstadd_front", t_lstadd_front);
	run("ft_lstsize", t_lstsize);
	run("ft_lstlast", t_lstlast);
	run("ft_lstadd_back", t_lstadd_back);
	run("ft_lstdelone", t_lstdelone);
	run("ft_lstclear", t_lstclear);
	run("ft_lstiter", t_lstiter);
	run("ft_lstmap", t_lstmap);
	section("RESULTAT");
	printf("  Fonctions OK : %s%d/%d" RST "\n",
		g_fn_ok == g_fn_total ? GRN : RED, g_fn_ok, g_fn_total);
	printf("  Tests reussis : %s%d/%d" RST "\n",
		g_ko == 0 ? GRN : RED, g_ok, g_ok + g_ko);
	printf("\n  Pense aussi a verifier les fuites memoire :\n"
		"      valgrind --leak-check=full ./test_libft\n\n");
	return (g_ko != 0);
}
