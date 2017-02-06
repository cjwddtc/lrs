#include "db_tools.h"
void select_(db_statement& db_st, db_attribute& db_at)
{
    db_st.stream << db_at.str();
}

void db_statement::from();