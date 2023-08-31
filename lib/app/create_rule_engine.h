    static std::shared_ptr<VariableStorage> load(std::shared_ptr<SDRUnit>&, std::shared_ptr<Module>&);
    static void load_unit(std::shared_ptr<SDRUnit>&, std::shared_ptr<VariableStorage>&);
    static void load_module(std::shared_ptr<Module>&, std::shared_ptr<VariableStorage>&);

        template <typename T>
    void RuleEngine::load_rules(std::shared_ptr<T>& src);

std::shared_ptr<VariableStorage> RuleEngine::load(std::shared_ptr<SDRUnit>& unit, std::shared_ptr<Module>& module) {
    auto storage = ps::make_shared<VariableStorage>();
    load_module(module, storage);
    load_unit(unit, storage);

    return storage;
}



void RuleEngine::load_module(std::shared_ptr<Module>& module, std::shared_ptr<VariableStorage>& storage) {

}


template <typename T>
void RuleEngine::load_rules(std::shared_ptr<T>& src) {
    auto& rules = src -> getRules();
    for (auto& rule : rules) {
        add_rule(
            rule.priority,
            rule.expression,
            rule.command
        );
    }
}

