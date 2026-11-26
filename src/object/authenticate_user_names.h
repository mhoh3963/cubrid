/*
 *
 * Copyright 2016 CUBRID Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

/*
 * authenticate_user_names.h - names of the built-in users
 *
 * Keep this header free of includes and types: it is also used by programs that see only
 * the CCI headers (broker_tester, cas_runner), whose types conflict with the engine types.
 */

#ifndef _AUTHENTICATE_USER_NAMES_H_
#define _AUTHENTICATE_USER_NAMES_H_

#define AU_PUBLIC_USER_NAME     "CMEMBER"
#define AU_DBA_USER_NAME        "CMASTER"

/* lower-case forms, used where a user name is part of a unique name (e.g. "cmaster.t1") */
#define AU_PUBLIC_USER_NAME_LOWER       "cmember"
#define AU_DBA_USER_NAME_LOWER          "cmaster"

/*
 * GRANT/REVOKE ... TO PUBLIC is accepted as an alias of AU_PUBLIC_USER_NAME.
 * The alias is a reserved word and can not be used as a user name.
 */
#define AU_PUBLIC_ALIAS_NAME    "PUBLIC"

/* minimum length (in characters) of a user name */
#define AU_MIN_USER_NAME_LENGTH 6

#endif /* _AUTHENTICATE_USER_NAMES_H_ */
