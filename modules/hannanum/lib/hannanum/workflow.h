// derived from Workflow/WorkflowFactory

/*  Copyright 2010, 2011 Semantic Web Research Center, KAIST

This file is part of JHanNanum.

JHanNanum is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

JHanNanum is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JHanNanum.  If not, see <http://www.gnu.org/licenses/>   */


#ifndef HANNANUM_WORKFLOW_H
#define HANNANUM_WORKFLOW_H

hannanum_t *hannanum_create(const hannanum_options_t *options);
void hannanum_destroy(hannanum_t *hannanum);
const char *hannanum_error(const hannanum_t *hannanum);
hannanum_result_t *hannanum_analyze(hannanum_t *hannanum, const char *input);
void hannanum_result_destroy(hannanum_result_t *result);
size_t hannanum_result_eojeol_count(const hannanum_result_t *result);
size_t hannanum_result_morpheme_count(const hannanum_result_t *result, size_t eojeol_index);
const char *hannanum_result_plain(const hannanum_result_t *result, size_t eojeol_index);
const char *hannanum_result_morpheme(const hannanum_result_t *result, size_t eojeol_index, size_t morpheme_index);
const char *hannanum_result_tag(const hannanum_result_t *result, size_t eojeol_index, size_t morpheme_index);
char *hannanum_result_format(const hannanum_result_t *result);

#endif
