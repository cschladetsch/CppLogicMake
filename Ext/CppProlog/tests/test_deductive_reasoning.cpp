#include <gtest/gtest.h>
#include "prolog/interpreter.h"

using namespace prolog;

class DeductiveReasoningTest : public ::testing::Test {
protected:
    std::unique_ptr<Interpreter> interpreter;
    
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>(false);
    }
    
    void TearDown() override {
        interpreter.reset();
    }
    
    void loadProgram(const std::string& program) {
        interpreter->loadString(program);
    }
    
    std::vector<Solution> query(const std::string& query_string) {
        return interpreter->query(query_string);
    }
    
    bool hasExactSolutions(const std::string& query_string, int expected_count) {
        auto solutions = query(query_string);
        return solutions.size() == static_cast<size_t>(expected_count);
    }
    
    bool hasSolutions(const std::string& query_string) {
        auto solutions = query(query_string);
        return !solutions.empty();
    }
    
    bool hasNoSolutions(const std::string& query_string) {
        auto solutions = query(query_string);
        return solutions.empty();
    }
};

// Test 1: Basic Modus Ponens (If A then B, A, therefore B)
TEST_F(DeductiveReasoningTest, ModusPonens) {
    loadProgram(R"(
        mortal(X) :- human(X).
        human(socrates).
    )");
    
    EXPECT_TRUE(hasSolutions("mortal(socrates)"));
    EXPECT_TRUE(hasNoSolutions("mortal(plato)"));
}

// Test 2: Syllogistic Reasoning (All A are B, All B are C, therefore All A are C)
TEST_F(DeductiveReasoningTest, SyllogisticReasoning) {
    loadProgram(R"(
        mortal(X) :- animal(X).
        animal(X) :- mammal(X).
        mammal(dog).
        mammal(cat).
    )");
    
    EXPECT_TRUE(hasSolutions("animal(dog)"));
    EXPECT_TRUE(hasSolutions("mortal(dog)"));
    EXPECT_TRUE(hasSolutions("mortal(cat)"));
    EXPECT_TRUE(hasNoSolutions("mortal(bird)"));
}

// Test 3: Transitive Relations
TEST_F(DeductiveReasoningTest, TransitiveRelations) {
    loadProgram(R"(
        ancestor(X, Y) :- parent(X, Y).
        ancestor(X, Z) :- parent(X, Y), ancestor(Y, Z).
        
        parent(adam, cain).
        parent(cain, enoch).
        parent(enoch, irad).
    )");
    
    EXPECT_TRUE(hasSolutions("ancestor(adam, cain)"));
    EXPECT_TRUE(hasSolutions("ancestor(adam, enoch)"));
    EXPECT_TRUE(hasSolutions("ancestor(adam, irad)"));
    EXPECT_TRUE(hasSolutions("ancestor(cain, irad)"));
    EXPECT_TRUE(hasNoSolutions("ancestor(irad, adam)"));
}

// Test 4: Contrapositive Reasoning (If A then B, not B, therefore not A)
TEST_F(DeductiveReasoningTest, ContrapositiveReasoning) {
    loadProgram(R"(
        flies(X) :- bird(X), can_fly(X).
        bird(penguin).
        bird(eagle).
        can_fly(eagle).
    )");
    
    EXPECT_TRUE(hasSolutions("flies(eagle)"));
    EXPECT_TRUE(hasNoSolutions("flies(penguin)"));
}

// Test 5: Disjunctive Reasoning (A or B, not A, therefore B)
TEST_F(DeductiveReasoningTest, DisjunctiveReasoning) {
    loadProgram(R"(
        solution(X) :- method_a(X).
        solution(X) :- method_b(X).
        
        method_b(problem1).
        method_a(problem2).
    )");
    
    EXPECT_TRUE(hasSolutions("solution(problem1)"));
    EXPECT_TRUE(hasSolutions("solution(problem2)"));
    EXPECT_TRUE(hasExactSolutions("solution(X)", 2));
}

// Test 6: Mathematical Induction Simulation
TEST_F(DeductiveReasoningTest, MathematicalInduction) {
    loadProgram(R"(
        natural(0).
        natural(1).
        natural(2).
        natural(3).
        
        successor(X, Y) :- natural(X), natural(Y), follows(X, Y).
        follows(0, 1).
        follows(1, 2).
        follows(2, 3).
    )");
    
    EXPECT_TRUE(hasSolutions("natural(0)"));
    EXPECT_TRUE(hasSolutions("natural(1)"));
    EXPECT_TRUE(hasSolutions("successor(0, 1)"));
}

// Test 7: Proof by Cases
TEST_F(DeductiveReasoningTest, ProofByCases) {
    loadProgram(R"(
        even_or_odd(X) :- even(X).
        even_or_odd(X) :- odd(X).
        
        even(0).
        even(2).
        even(4).
        odd(1).
        odd(3).
        odd(5).
    )");
    
    EXPECT_TRUE(hasSolutions("even_or_odd(0)"));
    EXPECT_TRUE(hasSolutions("even_or_odd(1)"));
    EXPECT_TRUE(hasSolutions("even_or_odd(2)"));
    EXPECT_TRUE(hasSolutions("even_or_odd(3)"));
}

// Test 8: Logical Equivalence (P iff Q means P->Q and Q->P)
TEST_F(DeductiveReasoningTest, LogicalEquivalence) {
    loadProgram(R"(
        equivalent(X, Y) :- implies(X, Y), implies(Y, X).
        implies(X, Y) :- conditional(X, Y).
        
        conditional(raining, wet_ground).
        conditional(wet_ground, raining).
    )");
    
    EXPECT_TRUE(hasSolutions("equivalent(raining, wet_ground)"));
}

// Test 9: De Morgan's Laws Simulation
TEST_F(DeductiveReasoningTest, DeMorgansLaws) {
    loadProgram(R"(
        not_both(X, Y) :- not_x(X).
        not_both(X, Y) :- not_y(Y).
        
        not_x(a).
        not_y(b).
        
        property(X) :- not_both(X, Y).
    )");
    
    EXPECT_TRUE(hasSolutions("property(a)"));
    EXPECT_TRUE(hasSolutions("property(b)"));
}

// Test 10: Hypothetical Syllogism (If A then B, If B then C, therefore If A then C)
TEST_F(DeductiveReasoningTest, HypotheticalSyllogism) {
    loadProgram(R"(
        conclusion(X) :- premise1(X).
        premise1(X) :- premise2(X).
        premise2(X) :- initial_fact(X).
        
        initial_fact(data).
    )");
    
    EXPECT_TRUE(hasSolutions("conclusion(data)"));
    EXPECT_TRUE(hasNoSolutions("conclusion(other)"));
}

// Test 11: Constructive Dilemma
TEST_F(DeductiveReasoningTest, ConstructiveDilemma) {
    loadProgram(R"(
        outcome(X) :- condition_a(X), result_a(X).
        outcome(X) :- condition_b(X), result_b(X).
        
        condition_a(case1).
        result_a(case1).
        condition_b(case2).
        result_b(case2).
    )");
    
    EXPECT_TRUE(hasSolutions("outcome(case1)"));
    EXPECT_TRUE(hasSolutions("outcome(case2)"));
}

// Test 12: Resolution Principle
TEST_F(DeductiveReasoningTest, ResolutionPrinciple) {
    loadProgram(R"(
        goal :- clause1, clause2.
        clause1 :- fact1.
        clause2 :- fact2.
        
        fact1.
        fact2.
    )");
    
    EXPECT_TRUE(hasSolutions("goal"));
}

// Test 13: Backward Chaining Inference
TEST_F(DeductiveReasoningTest, BackwardChaining) {
    loadProgram(R"(
        diagnose(flu) :- symptom(fever), symptom(cough), symptom(fatigue).
        symptom(fever) :- temperature(high).
        symptom(cough) :- throat(sore).
        symptom(fatigue) :- energy(low).
        
        temperature(high).
        throat(sore).
        energy(low).
    )");
    
    EXPECT_TRUE(hasSolutions("diagnose(flu)"));
}

// Test 14: Forward Chaining Inference
TEST_F(DeductiveReasoningTest, ForwardChaining) {
    loadProgram(R"(
        step1 :- initial_state.
        step2 :- step1.
        step3 :- step2.
        final_result :- step3.
        
        initial_state.
    )");
    
    EXPECT_TRUE(hasSolutions("step1"));
    EXPECT_TRUE(hasSolutions("step2"));
    EXPECT_TRUE(hasSolutions("step3"));
    EXPECT_TRUE(hasSolutions("final_result"));
}

// Test 15: Non-monotonic Reasoning Simulation
TEST_F(DeductiveReasoningTest, NonmonotonicReasoning) {
    loadProgram(R"(
        can_fly(X) :- bird(X).
        bird(tweety).
        bird(penguin).
        penguin(penguin).
    )");
    
    EXPECT_TRUE(hasSolutions("can_fly(tweety)"));
    EXPECT_TRUE(hasSolutions("can_fly(penguin)"));  // Default assumption
}

// Test 16: Abductive Reasoning
TEST_F(DeductiveReasoningTest, AbductiveReasoning) {
    loadProgram(R"(
        explains(Theory, Observation) :- causes(Theory, Observation).
        causes(rain, wet_grass).
        causes(sprinkler, wet_grass).
        
        observation(wet_grass).
        plausible_theory(X) :- causes(X, wet_grass).
    )");
    
    EXPECT_TRUE(hasSolutions("plausible_theory(rain)"));
    EXPECT_TRUE(hasSolutions("plausible_theory(sprinkler)"));
}

// Test 17: Analogical Reasoning
TEST_F(DeductiveReasoningTest, AnalogicalReasoning) {
    loadProgram(R"(
        similar_structure(X, Y) :- has_property(X, P), has_property(Y, P).
        transfer_property(X, Y, Q) :- similar_structure(X, Y), has_property(X, Q).
        
        has_property(earth, planet).
        has_property(mars, planet).
        has_property(earth, supports_life).
        
        might_have(Y, Q) :- transfer_property(earth, Y, Q).
    )");
    
    EXPECT_TRUE(hasSolutions("similar_structure(earth, mars)"));
    EXPECT_TRUE(hasSolutions("might_have(mars, supports_life)"));
}

// Test 18: Causal Reasoning
TEST_F(DeductiveReasoningTest, CausalReasoning) {
    loadProgram(R"(
        effect(Y) :- cause(X), leads_to(X, Y).
        leads_to(fire, smoke).
        leads_to(rain, wet_ground).
        
        cause(fire).
        observable(X) :- effect(X).
    )");
    
    EXPECT_TRUE(hasSolutions("effect(smoke)"));
    EXPECT_TRUE(hasSolutions("observable(smoke)"));
}

// Test 19: Counterfactual Reasoning
TEST_F(DeductiveReasoningTest, CounterfactualReasoning) {
    loadProgram(R"(
        would_happen(Y) :- if_condition(X), then_result(X, Y).
        if_condition(study_hard).
        then_result(study_hard, good_grades).
        
        actual_outcome(X) :- would_happen(X), condition_met(study_hard).
        condition_met(study_hard).
    )");
    
    EXPECT_TRUE(hasSolutions("would_happen(good_grades)"));
    EXPECT_TRUE(hasSolutions("actual_outcome(good_grades)"));
}

// Test 20: Meta-logical Reasoning (Reasoning about Logic)
TEST_F(DeductiveReasoningTest, MetaLogicalReasoning) {
    loadProgram(R"(
        valid_inference(Rule) :- sound_rule(Rule), correct_premises(Rule).
        sound_rule(modus_ponens).
        correct_premises(modus_ponens).
        
        logical_conclusion(X) :- valid_inference(modus_ponens), derives(modus_ponens, X).
        derives(modus_ponens, socrates_mortal).
        
        meta_valid(X) :- logical_conclusion(X).
    )");
    
    EXPECT_TRUE(hasSolutions("valid_inference(modus_ponens)"));
    EXPECT_TRUE(hasSolutions("logical_conclusion(socrates_mortal)"));
    EXPECT_TRUE(hasSolutions("meta_valid(socrates_mortal)"));
}

// Bonus Test 21: Complex Multi-level Inference
TEST_F(DeductiveReasoningTest, ComplexMultiLevelInference) {
    loadProgram(R"(
        % Knowledge hierarchy
        entity(X) :- living(X).
        living(X) :- animal(X).
        living(X) :- plant(X).
        animal(X) :- mammal(X).
        animal(X) :- bird(X).
        mammal(X) :- carnivore(X).
        mammal(X) :- herbivore(X).
        
        % Behavioral rules
        hunts(X) :- carnivore(X), predator(X).
        eats_plants(X) :- herbivore(X).
        
        % Ecosystem rules
        food_chain(X, Y) :- hunts(X), prey(Y), eats_plants(Y).
        
        % Facts
        carnivore(lion).
        predator(lion).
        herbivore(gazelle).
        prey(gazelle).
        
        % High-level inference
        ecosystem_balance :- food_chain(X, Y), carnivore(X), herbivore(Y).
    )");
    
    EXPECT_TRUE(hasSolutions("entity(lion)"));
    EXPECT_TRUE(hasSolutions("living(lion)"));
    EXPECT_TRUE(hasSolutions("animal(lion)"));
    EXPECT_TRUE(hasSolutions("mammal(lion)"));
    EXPECT_TRUE(hasSolutions("hunts(lion)"));
    EXPECT_TRUE(hasSolutions("food_chain(lion, gazelle)"));
    EXPECT_TRUE(hasSolutions("ecosystem_balance"));
}

// Bonus Test 22: Proof by Contradiction Simulation
TEST_F(DeductiveReasoningTest, ProofByContradiction) {
    loadProgram(R"(
        assume_not_prime(2).
        
        composite(X) :- has_divisor(X, Y).
        
        contradiction :- assume_not_prime(2), composite(2).
        
        prime(2) :- assume_not_prime(2), cannot_be_composite(2).
        cannot_be_composite(X) :- integer(X).
        integer(2).
    )");
    
    EXPECT_TRUE(hasSolutions("cannot_be_composite(2)"));
    EXPECT_TRUE(hasSolutions("prime(2)"));
    EXPECT_TRUE(hasNoSolutions("composite(2)"));
}