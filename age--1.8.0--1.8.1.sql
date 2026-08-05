DROP FUNCTION ag_catalog.agtype_access_operator(VARIADIC agtype[]);

CREATE FUNCTION ag_catalog.agtype_access_operator(VARIADIC "any")
RETURNS agtype
LANGUAGE c
IMMUTABLE
RETURNS NULL ON NULL INPUT
PARALLEL SAFE
AS 'MODULE_PATHNAME';