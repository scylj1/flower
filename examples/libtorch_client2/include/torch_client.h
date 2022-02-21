/***********************************************************************************************************
 *
 * @file libtorch_client.h
 *
 * @brief Define an example flower client, train and test method
 * 
 * @version 1.0
 * 
 * @date 06/09/2021
 *
 * ********************************************************************************************************/

#pragma once
#include "client.h"
#include <iostream>
#include <torch/torch.h>
#include <torchvision/vision.h>
#include <torchvision/models/resnet.h>
#include "cifar10.h"
#include <ctime>

/**
 * Validate the network on the entire test set
 *
 */
template <typename DataLoader>
std::tuple<size_t, float, double> test(vision::models::ResNet18& net,
    DataLoader& test_loader,
    torch::Device device)
{
    net->to(device);
    net->eval();
    size_t num_samples = 0;
    size_t num_correct = 0;
    float running_loss = 0.0;

    // Iterate the data loader to yield batches from the dataset.
    for (auto& batch : *test_loader)
    {
        auto data = batch.data.to(device), target = batch.target.to(device);
        num_samples += data.size(0);
        // Execute the model on the input data.
        torch::Tensor output = net->forward(data);

        // Compute a loss value to judge the prediction of our model.
        torch::Tensor loss = torch::nn::functional::cross_entropy(output, target);

        AT_ASSERT(!std::isnan(loss.template item<float>()));
        running_loss += loss.item<float>() * data.size(0); // CHECK IF IT IS DOUBLE OR FLOAT!!!!
        auto prediction = output.argmax(1);
        num_correct += prediction.eq(target).sum().template item<int64_t>();
    }
    auto mean_loss = running_loss / num_samples;
    auto accuracy = static_cast<double>(num_correct) / num_samples;

    std::cout << "Testset - Loss: " << mean_loss << ", Accuracy: " << accuracy << std::endl;

    return std::make_tuple(num_samples, running_loss, accuracy);
}

/**
 * Train the net work on the training set
 */
template <typename DataLoader>
std::tuple<size_t, float, double> train(vision::models::ResNet18& net, 
    DataLoader& train_loader,
    torch::optim::Optimizer& optimizer,
    torch::Device device)
{
    net->to(device);
    net->train();
    size_t num_samples = 0;
    size_t num_correct = 0;
    float running_loss = 0.0;

    // Iterate the data loader to yield batches from the dataset.
    for (auto& batch : *train_loader){
        auto data = batch.data.to(device), target = batch.target.to(device);
        num_samples += data.size(0);
        // Reset gradients.
        optimizer.zero_grad();

        // Execute the model on the input data.
        torch::Tensor output = net->forward(data);

        // Compute a loss value to judge the prediction of our model.
        torch::Tensor loss = torch::nn::functional::cross_entropy(output, target);

        AT_ASSERT(!std::isnan(loss.template item<float>()));
        //std::cout << loss.item<float>() << std::endl;
        running_loss += loss.item<float>() * data.size(0); // CHECK IF IT IS DOUBLE OR FLOAT!!!!
        auto prediction = output.argmax(1);
        num_correct += prediction.eq(target).sum().template item<int64_t>();

        // Compute gradients of the loss w.r.t. the parameters of our model.
        loss.backward();
        // Update the parameters based on the calculated gradients.
        optimizer.step();
    }
    auto mean_loss = running_loss / num_samples;
    auto accuracy = static_cast<double>(num_correct) / num_samples;

    return std::make_tuple(num_samples, mean_loss, accuracy);
}

/**
 * A concrete libtorch flower example extends from flwr::Client abstract class
 * Uses ResNet18 as base model
 *
 **/

template<typename DataLoader>
class TorchClient : public flwr::Client {
  private:
    vision::models::ResNet18& net;
    DataLoader& train_loader;
    DataLoader& test_loader;
    torch::optim::Optimizer& optimizer;
    torch::Device device;
    int64_t client_id;

  public:
     
    TorchClient(
		    std::string client_id,
		    vision::models::ResNet18& net,
		    DataLoader& train_loader,
		    DataLoader& test_loader,
		    torch::optim::Optimizer& optimizer,
		    torch::Device device) : net(net), train_loader(train_loader), test_loader(test_loader), optimizer(optimizer), device(device){};

/**
 * Return the current local model parameters
 * Simple string are used for now to test communication, needs updates in the future
 */
    virtual flwr::ParametersRes get_parameters() override {
      // Serialize
      std::ostringstream stream;
      torch::save(net, stream);
      std::list<std::string> tensors;
      tensors.push_back(stream.str());
      std::string tensor_str = "libtorch";
      return flwr::Parameters(tensors, tensor_str); 
    };

    virtual flwr::PropertiesRes get_properties(flwr::PropertiesIns ins) override {
      flwr::PropertiesRes p;
      p.setPropertiesRes(static_cast<flwr::Properties>(ins.getPropertiesIns()));
      return p;
    };

/**
 * Refine the provided weights using the locally held dataset
 * Simple settings are used for testing, needs updates in the future
 */
    virtual flwr::FitRes fit(flwr::FitIns ins) override{
      flwr::FitRes resp;
      flwr::Parameters p = ins.getParameters();
      std::list<std::string> s = p.getTensors();
      //std::cout << s.empty() << std::endl;
      if (s.empty() == 0){
        std::istringstream stream(s.front());  
        torch::load(net, stream);
      }
      std::tuple<size_t, float, double> result = train(net, train_loader, optimizer, device);
      resp.setParameters(this->get_parameters().getParameters());
      resp.setNum_example(std::get<0>(result));
      //resp.setNum_example(30);
  
      return resp;
    };

/**
 * Evaluate the provided weights using the locally held dataset
 * Needs updates in the future
 */
    virtual flwr::EvaluateRes evaluate(flwr::EvaluateIns ins) override {
      flwr::EvaluateRes resp;
      flwr::Parameters p = ins.getParameters();
      std::list<std::string> s = p.getTensors();
      //std::cout << s.empty() << std::endl;
      if (s.empty() == 0){
        std::istringstream stream(s.front());
        torch::load(net, stream);
      }
      std::tuple<size_t, float, double> result = test(net, test_loader, device);
      resp.setNum_example(std::get<0>(result));
      resp.setLoss(std::get<1>(result));
      //resp.setNum_example(30);
      //resp.setLoss(0.1);
 
      return resp;
    };
};
