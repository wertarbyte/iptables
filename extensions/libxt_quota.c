/*
 * Shared library add-on to iptables to add quota support
 *
 * Sam Johnston <samj@samj.net>
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>

#include <linux/netfilter/xt_quota.h>

static const struct option quota_opts[] = {
	{.name = "quota", .has_arg = true, .val = '1'},
	XT_GETOPT_TABLEEND,
};

static void quota_help(void)
{
	printf("quota match options:\n"
	       "[!] --quota quota		quota (bytes)\n");
}

static void
quota_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_quota_info *q = (const void *)match->data;
	printf(" quota: %llu bytes", (unsigned long long)q->quota);
}

static void
quota_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_quota_info *q = (const void *)match->data;

	if (q->flags & XT_QUOTA_INVERT)
		printf("! ");
	printf(" --quota %llu", (unsigned long long) q->quota);
}

/* parse quota option */
static int
parse_quota(const char *s, uint64_t * quota)
{
	*quota = strtoull(s, NULL, 10);

#ifdef DEBUG_XT_QUOTA
	printf("Quota: %llu\n", *quota);
#endif

	if (*quota == UINT64_MAX)
		xtables_error(PARAMETER_PROBLEM, "quota invalid: '%s'\n", s);
	else
		return 1;
}

static int
quota_parse(int c, char **argv, int invert, unsigned int *flags,
	    const void *entry, struct xt_entry_match **match)
{
	struct xt_quota_info *info = (struct xt_quota_info *) (*match)->data;

	switch (c) {
	case '1':
		if (xtables_check_inverse(optarg, &invert, NULL, 0, argv))
			xtables_error(PARAMETER_PROBLEM, "quota: unexpected '!'");
		if (!parse_quota(optarg, &info->quota))
			xtables_error(PARAMETER_PROBLEM,
				   "bad quota: '%s'", optarg);

		if (invert)
			info->flags |= XT_QUOTA_INVERT;
		*flags |= 1;
		break;
	}
	return 1;
}

static void quota_check(unsigned int flags)
{
	if (flags == 0)
		xtables_error(PARAMETER_PROBLEM,
			"quota: the --quota argument must be specified\n");
}

static struct xtables_match quota_match = {
	.family		= NFPROTO_UNSPEC,
	.name		= "quota",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof (struct xt_quota_info)),
	.userspacesize	= offsetof(struct xt_quota_info, master),
	.help		= quota_help,
	.parse		= quota_parse,
	.final_check	= quota_check,
	.print		= quota_print,
	.save		= quota_save,
	.extra_opts	= quota_opts,
};

void
_init(void)
{
	xtables_register_match(&quota_match);
}
