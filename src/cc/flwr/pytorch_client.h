/*
* A self-defined class for testing message handling
*
*/
class Example_client : public Client {
public:
    virtual ParametersRes get_parameters() override {
        //std::cout << "DEBUG: get parameters" << std::endl;
        std::list<std::string> tensors;
        tensors.push_back("1");
        tensors.push_back("2");
        tensors.push_back("3");
        Parameters p(tensors, "example tensor");

        return ParametersRes(p);
    }

    virtual FitRes fit(FitIns ins) override {
        std::list<std::string> tensors;
        for (auto& i : ins.getParameters().getTensors()) {
            tensors.push_back(i + "1");
        }
        Metrics m;
        for (auto& i : ins.getConfig()) {
            m[i.first + "1"] = i.second;
        }

        return FitRes(Parameters(tensors, ins.getParameters().getTensor_type()), 5, 1, 10, m);
    }

    virtual EvaluateRes evaluate(EvaluateIns ins) override {
        Metrics m;
        for (auto& i : ins.getConfig()) {
            m[i.first + "2"] = i.second;
        }

        return EvaluateRes(0.5, 5, 0.9, m);
    }
};
