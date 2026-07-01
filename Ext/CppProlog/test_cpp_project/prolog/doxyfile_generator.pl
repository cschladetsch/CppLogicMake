% Doxyfile Generator using Prolog
% Generates optimized Doxyfile for Rust documentation using declarative rules

% ============================================================================
% Doxygen Configuration Templates
% ============================================================================

% Project information rules
project_info(rust_hello_world, ProjectInfo) :-
    ProjectInfo = [
        config_line('PROJECT_NAME', '"Hello World Rust Application"'),
        config_line('PROJECT_NUMBER', '"0.1.0"'),
        config_line('PROJECT_BRIEF', '"Generated Hello World application using CppProlog and Rust"'),
        config_line('PROJECT_LOGO', ''),
        config_line('OUTPUT_DIRECTORY', './docs'),
        config_line('CREATE_SUBDIRS', 'NO')
    ].

% Input configuration for Rust projects
input_config(rust_project, InputConfig) :-
    InputConfig = [
        config_line('INPUT', 'src/'),
        config_line('INPUT_ENCODING', 'UTF-8'),
        config_line('FILE_PATTERNS', '*.rs *.md'),
        config_line('RECURSIVE', 'YES'),
        config_line('EXCLUDE', 'target/'),
        config_line('EXCLUDE_SYMLINKS', 'NO'),
        config_line('EXCLUDE_PATTERNS', '*/target/*'),
        config_line('EXCLUDE_SYMBOLS', ''),
        config_line('EXAMPLE_PATH', 'examples/'),
        config_line('EXAMPLE_PATTERNS', '*.rs'),
        config_line('EXAMPLE_RECURSIVE', 'NO')
    ].

% Output format configuration
output_config(standard, OutputConfig) :-
    OutputConfig = [
        config_line('GENERATE_HTML', 'YES'),
        config_line('HTML_OUTPUT', 'html'),
        config_line('HTML_FILE_EXTENSION', '.html'),
        config_line('HTML_HEADER', ''),
        config_line('HTML_FOOTER', ''),
        config_line('HTML_STYLESHEET', ''),
        config_line('HTML_EXTRA_STYLESHEET', ''),
        config_line('HTML_EXTRA_FILES', ''),
        config_line('HTML_COLORSTYLE_HUE', '220'),
        config_line('HTML_COLORSTYLE_SAT', '100'),
        config_line('HTML_COLORSTYLE_GAMMA', '80'),
        config_line('HTML_TIMESTAMP', 'YES'),
        config_line('HTML_DYNAMIC_SECTIONS', 'NO'),
        config_line('HTML_INDEX_NUM_ENTRIES', '100'),
        config_line('GENERATE_DOCSET', 'NO'),
        config_line('GENERATE_HTMLHELP', 'NO'),
        config_line('GENERATE_QHP', 'NO'),
        config_line('GENERATE_ECLIPSEHELP', 'NO'),
        config_line('DISABLE_INDEX', 'NO'),
        config_line('GENERATE_TREEVIEW', 'NO'),
        config_line('ENUM_VALUES_PER_LINE', '4'),
        config_line('TREEVIEW_WIDTH', '250'),
        config_line('EXT_LINKS_IN_WINDOW', 'NO')
    ].

% LaTeX output configuration
latex_config(disabled, LatexConfig) :-
    LatexConfig = [
        config_line('GENERATE_LATEX', 'NO'),
        config_line('LATEX_OUTPUT', 'latex'),
        config_line('LATEX_CMD_NAME', 'latex'),
        config_line('MAKEINDEX_CMD_NAME', 'makeindex'),
        config_line('COMPACT_LATEX', 'NO'),
        config_line('PAPER_TYPE', 'a4'),
        config_line('EXTRA_PACKAGES', ''),
        config_line('LATEX_HEADER', ''),
        config_line('LATEX_FOOTER', ''),
        config_line('LATEX_EXTRA_STYLESHEET', ''),
        config_line('LATEX_EXTRA_FILES', ''),
        config_line('PDF_HYPERLINKS', 'YES'),
        config_line('USE_PDFLATEX', 'YES'),
        config_line('LATEX_BATCHMODE', 'NO'),
        config_line('LATEX_HIDE_INDICES', 'NO'),
        config_line('LATEX_SOURCE_CODE', 'NO'),
        config_line('LATEX_BIB_STYLE', 'plain')
    ].

% Documentation extraction settings
extraction_config(rust_standard, ExtractionConfig) :-
    ExtractionConfig = [
        config_line('EXTRACT_ALL', 'YES'),
        config_line('EXTRACT_PRIVATE', 'NO'),
        config_line('EXTRACT_PACKAGE', 'NO'),
        config_line('EXTRACT_STATIC', 'YES'),
        config_line('EXTRACT_LOCAL_CLASSES', 'YES'),
        config_line('EXTRACT_LOCAL_METHODS', 'YES'),
        config_line('EXTRACT_ANON_NSPACES', 'NO'),
        config_line('HIDE_UNDOC_MEMBERS', 'NO'),
        config_line('HIDE_UNDOC_CLASSES', 'NO'),
        config_line('HIDE_FRIEND_COMPOUNDS', 'NO'),
        config_line('HIDE_IN_BODY_DOCS', 'NO'),
        config_line('INTERNAL_DOCS', 'NO'),
        config_line('CASE_SENSE_NAMES', 'YES'),
        config_line('HIDE_SCOPE_NAMES', 'NO'),
        config_line('HIDE_COMPOUND_REFERENCE', 'NO'),
        config_line('SHOW_INCLUDE_FILES', 'YES'),
        config_line('SHOW_GROUPED_MEMB_INC', 'NO'),
        config_line('FORCE_LOCAL_INCLUDES', 'NO'),
        config_line('INLINE_INFO', 'YES'),
        config_line('SORT_MEMBER_DOCS', 'YES'),
        config_line('SORT_BRIEF_DOCS', 'NO'),
        config_line('SORT_MEMBERS_CTORS_1ST', 'NO'),
        config_line('SORT_GROUP_NAMES', 'NO'),
        config_line('SORT_BY_SCOPE_NAME', 'NO')
    ].

% Build-related settings
build_config(standard, BuildConfig) :-
    BuildConfig = [
        config_line('QUIET', 'NO'),
        config_line('WARNINGS', 'YES'),
        config_line('WARN_IF_UNDOCUMENTED', 'YES'),
        config_line('WARN_IF_DOC_ERROR', 'YES'),
        config_line('WARN_NO_PARAMDOC', 'NO'),
        config_line('WARN_AS_ERROR', 'NO'),
        config_line('WARN_FORMAT', '"$file:$line: $text"'),
        config_line('WARN_LOGFILE', ''),
        config_line('ALPHABETICAL_INDEX', 'YES'),
        config_line('COLS_IN_ALPHA_INDEX', '5'),
        config_line('IGNORE_PREFIX', ''),
        config_line('GENERATE_TODOLIST', 'YES'),
        config_line('GENERATE_TESTLIST', 'YES'),
        config_line('GENERATE_BUGLIST', 'YES'),
        config_line('GENERATE_DEPRECATEDLIST', 'YES'),
        config_line('ENABLED_SECTIONS', ''),
        config_line('MAX_INITIALIZER_LINES', '30'),
        config_line('SHOW_USED_FILES', 'YES'),
        config_line('SHOW_FILES', 'YES'),
        config_line('SHOW_NAMESPACES', 'YES'),
        config_line('FILE_VERSION_FILTER', ''),
        config_line('LAYOUT_FILE', ''),
        config_line('CITE_BIB_FILES', '')
    ].

% ============================================================================
% Doxyfile Generation Engine
% ============================================================================

% Main predicate to generate complete Doxyfile
generate_doxyfile(ProjectType) :-
    write('# Doxyfile generated by Prolog-based Rust Generator'), nl,
    write('# Project: '), write(ProjectType), nl,
    write('# Generated on: '), get_time(Time), format_time(atom(TimeStr), '%Y-%m-%d %H:%M:%S', Time), write(TimeStr), nl,
    write('#---------------------------------------------------------------------------'), nl,
    write('# Project related configuration options'), nl,
    write('#---------------------------------------------------------------------------'), nl,
    project_info(ProjectType, ProjectConfig),
    generate_config_section(ProjectConfig),
    nl,
    write('#---------------------------------------------------------------------------'), nl,
    write('# Build related configuration options'), nl,
    write('#---------------------------------------------------------------------------'), nl,
    build_config(standard, BuildConfig),
    generate_config_section(BuildConfig),
    nl,
    write('#---------------------------------------------------------------------------'), nl,
    write('# Configuration options related to warning and progress messages'), nl,
    write('#---------------------------------------------------------------------------'), nl,
    extraction_config(rust_standard, ExtractionConfig),
    generate_config_section(ExtractionConfig),
    nl,
    write('#---------------------------------------------------------------------------'), nl,
    write('# Configuration options related to the input files'), nl,
    write('#---------------------------------------------------------------------------'), nl,
    input_config(rust_project, InputConfig),
    generate_config_section(InputConfig),
    nl,
    write('#---------------------------------------------------------------------------'), nl,
    write('# Configuration options related to the HTML output'), nl,
    write('#---------------------------------------------------------------------------'), nl,
    output_config(standard, OutputConfig),
    generate_config_section(OutputConfig),
    nl,
    write('#---------------------------------------------------------------------------'), nl,
    write('# Configuration options related to the LaTeX output'), nl,
    write('#---------------------------------------------------------------------------'), nl,
    latex_config(disabled, LatexConfig),
    generate_config_section(LatexConfig).

% Generate configuration sections
generate_config_section([]).
generate_config_section([ConfigLine|Rest]) :-
    generate_config_line(ConfigLine),
    generate_config_section(Rest).

% Generate individual configuration lines
generate_config_line(config_line(Key, Value)) :-
    write(Key), write(' = '), write(Value), nl.

% Helper to format configuration with comments
generate_config_with_comment(config_line(Key, Value), Comment) :-
    write('# '), write(Comment), nl,
    write(Key), write(' = '), write(Value), nl.

% ============================================================================
% Rust-Specific Doxygen Optimizations
% ============================================================================

% Optimize Doxygen settings for Rust documentation
rust_optimizations(RustOptimizations) :-
    RustOptimizations = [
        config_line('OPTIMIZE_OUTPUT_FOR_C', 'NO'),
        config_line('OPTIMIZE_OUTPUT_JAVA', 'NO'),
        config_line('OPTIMIZE_FOR_FORTRAN', 'NO'),
        config_line('OPTIMIZE_OUTPUT_VHDL', 'NO'),
        config_line('OPTIMIZE_OUTPUT_SLICE', 'NO'),
        config_line('EXTENSION_MAPPING', 'rs=C++'),
        config_line('MARKDOWN_SUPPORT', 'YES'),
        config_line('TOC_INCLUDE_HEADINGS', '5'),
        config_line('AUTOLINK_SUPPORT', 'YES'),
        config_line('BUILTIN_STL_SUPPORT', 'NO'),
        config_line('CPP_CLI_SUPPORT', 'NO'),
        config_line('SIP_SUPPORT', 'NO'),
        config_line('IDL_PROPERTY_SUPPORT', 'YES'),
        config_line('DISTRIBUTE_GROUP_DOC', 'NO'),
        config_line('GROUP_NESTED_COMPOUNDS', 'NO'),
        config_line('SUBGROUPING', 'YES'),
        config_line('INLINE_GROUPED_CLASSES', 'NO'),
        config_line('INLINE_SIMPLE_STRUCTS', 'NO'),
        config_line('TYPEDEF_HIDES_STRUCT', 'NO')
    ].

% Generate enhanced Doxyfile with Rust optimizations
generate_enhanced_doxyfile(ProjectType) :-
    generate_doxyfile(ProjectType),
    nl,
    write('#---------------------------------------------------------------------------'), nl,
    write('# Rust-specific optimizations'), nl,
    write('#---------------------------------------------------------------------------'), nl,
    rust_optimizations(RustOpts),
    generate_config_section(RustOpts).

% ============================================================================
% Utility Predicates
% ============================================================================

% Helper to validate configuration
validate_doxyfile_config(Config) :-
    forall(member(config_line(Key, Value), Config),
           (atom(Key), nonvar(Value))).

% Helper to estimate documentation size
estimate_doc_size(NumFiles, EstimatedSizeMB) :-
    EstimatedSizeMB is NumFiles * 2 + 10.

% ============================================================================
% Example Queries
% ============================================================================

% To generate a basic Doxyfile:
% ?- generate_doxyfile(rust_hello_world).

% To generate an enhanced Doxyfile with Rust optimizations:
% ?- generate_enhanced_doxyfile(rust_hello_world).