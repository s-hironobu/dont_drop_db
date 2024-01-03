/*-------------------------------------------------------------------------
 * dont_drop_db.c
 *
 * This extension prohibits to drop the databases listed
 * in the dont_drop_db.list parameter.
 *
 * Author: suzuki hironobu (hironobu@interdb.jp) 1, Dec, 2019
 * Copyright (C) 2019-2024,  suzuki hironobu
 *-------------------------------------------------------------------------
 */
#include "postgres.h"
#include "miscadmin.h"
#include "tcop/utility.h"
#include "port.h"
#include "string.h"
#include "pg_config_manual.h"

PG_MODULE_MAGIC;

/* Saved hook values in case of unload */
static ProcessUtility_hook_type prev_ProcessUtility = NULL;

/* Function declarations */
void		_PG_init(void);
void		_PG_fini(void);

/* Static function declarations */
static void ddd_ProcessUtility(PlannedStmt *pstmt, const char *queryString,
#if PG_VERSION_NUM >= 140000
							   bool readOnlyTree,
#endif
							   ProcessUtilityContext context, ParamListInfo params,
							   QueryEnvironment *queryEnv, DestReceiver *dest,
#if PG_VERSION_NUM >= 130000
							   QueryCompletion *qc);
#else
							   char *completionTag);
#endif
static bool check_drop_database_statement(DropdbStmt *dstmt);
static bool check_drop_database(PlannedStmt *pstmt);

/* Static variable */
static char *ddd_list;

/* Module callback */
void
_PG_init(void)
{
	if (!process_shared_preload_libraries_in_progress)
		return;

	DefineCustomStringVariable("dont_drop_db.list",
							   "List of databases that you don't want to drop.",
							   NULL,
							   &ddd_list,
							   "postgres,template0,template1",
							   PGC_SIGHUP,
							   0,
							   NULL,
							   NULL,
							   NULL);

	EmitWarningsOnPlaceholders("dont_drop_db");

	/* Install hook. */
	prev_ProcessUtility = ProcessUtility_hook;
	ProcessUtility_hook = ddd_ProcessUtility;
}

void
_PG_fini(void)
{
	/* Uninstall hook. */
	ProcessUtility_hook = prev_ProcessUtility;
}

/* Compare the specific database name and the database names listed in the dont_drop_db.list. */
static bool
check_drop_database_statement(DropdbStmt *dstmt)
{
	char		buff[NAMEDATALEN];
	int			i,
				j;
	char		c;

	i = j = 0;
	buff[0] = '\0';

	for (i = 0; i < strlen(ddd_list); i++)
	{
		c = ddd_list[i];

		/* skip space */
		if (c == ' ' || c == '\t' || c == '\n' || c == '\v'
			|| c == '\f' || c == '\r')
			continue;

		if (c == ',' && j <= NAMEDATALEN - 1)
		{
			buff[j] = '\0';
			if (pg_strcasecmp(dstmt->dbname, buff) == 0
				|| pg_strcasecmp(buff, "ALL") == 0)
				return true;
			else
				j = 0;
		}
		else if (j <= NAMEDATALEN - 1)
		{
			buff[j++] = c;
		}
		else
		{
			/*
			 * Skip to the next entry because this dbname is invalid (too
			 * long).
			 */
			j = 0;
			buff[0] = '\0';
			while (c != ',' && c != '\0'	/* Skip to ',' or '\0'. */
				   && (i < strlen(ddd_list) - 1))
				c = ddd_list[++i];
		}
	}

	if (NAMEDATALEN - 1 < j)
		return false;

	buff[j] = '\0';
	if (strlen(buff) > 0)
		if (pg_strcasecmp(dstmt->dbname, buff) == 0
			|| pg_strcasecmp(buff, "ALL") == 0)
			return true;

	return false;
}

/* Prohibit to drop the databases listed in the dont_drop_db.list. */
static bool
check_drop_database(PlannedStmt *pstmt)
{
	switch (nodeTag(pstmt->utilityStmt))
	{
		case T_DropdbStmt:
			return check_drop_database_statement((DropdbStmt *) pstmt->utilityStmt);
			break;
		default:
			return false;
	}
}

/* ProcessUtility hook */
static void
			ddd_ProcessUtility(PlannedStmt *pstmt, const char *queryString,
#if PG_VERSION_NUM >= 140000
							   bool readOnlyTree,
#endif
							   ProcessUtilityContext context, ParamListInfo params,
							   QueryEnvironment *queryEnv, DestReceiver *dest,
#if PG_VERSION_NUM >= 130000
							   QueryCompletion *qc)
#else
							   char *completionTag)
#endif
{
	if (check_drop_database(pstmt))
	{
		ereport(ERROR,
				(errmsg("%s cannot be dropped.",
						(char *) ((DropdbStmt *) pstmt->utilityStmt)->dbname)));
		return;
	}

	if (prev_ProcessUtility)
		prev_ProcessUtility(pstmt, queryString,
#if PG_VERSION_NUM >= 140000
							readOnlyTree,
#endif
							context,
#if PG_VERSION_NUM >= 130000
							params, queryEnv, dest, qc);
#else
							params, queryEnv, dest, completionTag);
#endif
	else
		standard_ProcessUtility(pstmt, queryString,
#if PG_VERSION_NUM >= 140000
								readOnlyTree,
#endif
								context,
#if PG_VERSION_NUM >= 130000
								params, queryEnv, dest, qc);
#else
								params, queryEnv, dest, completionTag);
#endif
}
