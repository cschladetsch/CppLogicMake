% --- Facts: Direct Dependencies ---
% defines that ComponentA directly depends on ComponentB.
depends_on(webapp, api_client).
depends_on(webapp, auth_module).
depends_on(api_client, http_library).
depends_on(auth_module, http_library).
depends_on(auth_module, crypto_library).
depends_on(http_library, socket_library).

% --- Rules: Complex Relationships ---

% Rule to find all direct and indirect (transitive) dependencies.
% A component needs a dependency if it depends on it directly.
needs_component(Component, Dependency) :-
    depends_on(Component, Dependency).

% Or if it depends on an intermediate component that, in turn, needs the dependency.
needs_component(Component, Dependency) :-
    depends_on(Component, Intermediate),
    needs_component(Intermediate, Dependency).

% Rule to find which components use a given dependency.
is_dependency_for(Dependency, Component) :-
    needs_component(Component, Dependency).

% Rule to identify shared dependencies (used by more than one component directly).
shared_dependency(Dependency) :-
    depends_on(_, Dependency).
