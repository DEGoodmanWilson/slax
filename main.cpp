#include <iostream>
#include <sstream>
#include <luna/luna.h>
#include <json/json.h>
#include <cpr/cpr.h>
#include "args.hxx"

int main(int argc, char **argv)
{
    std::stringstream info;
    info << "slax " << SLAX_VERSION << " : " << "A server for observing Slack events over the Slack Events API.";
    args::ArgumentParser parser(info.str(), "Y'all have a nice day.");
    args::Positional<std::string> verification_token{parser, "verification token", "The token that Slack includes in every request it makes. Required"};
    args::ValueFlag<std::string> forward_url{parser, "forwarding URL", "Remote endpoint to forward the event to", {'f', "forward"}};
    args::ValueFlag<std::string> endpoint{parser, "endpoint", "Endpoint to serve. Default is \"/\"", {'e', "endpoint"}, "/"};
    args::ValueFlag<int> port{parser, "port", "Port to serve on. Default is 8023", {'p', "port"}, 8023};
    args::HelpFlag help{parser, "help", "Display this help menu", {'h', "help"}};

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    if (!verification_token)
    {
        std::cerr << "Please pass the verification token as provided by Slack" << std::endl;
        std::cerr << parser;

        return 1;
    }

    std::cout << "http://localhost" <<":" << args::get(port) << args::get(endpoint) << std::endl;


    luna::server server{luna::server::port{static_cast<uint16_t>(args::get(port))}};
    if (!server)
    {
        std::cerr << "Failed to stand up server! Aborting" << std::endl;
        return 2;
    }

    server.handle_request(luna::request_method::POST, args::get(endpoint), [&verification_token, &forward_url](const luna::request &req) -> luna::response
    {
        // First off, forward the request
        if (forward_url)
        {
            auto result = cpr::Post(cpr::Url{args::get(forward_url)},
                      cpr::Body{req.body}
            );
        }

        Json::Reader reader;
        Json::Value envelope_obj;

        bool parsed_success = reader.parse(req.body, envelope_obj, false);
        if(!parsed_success)
        {
            std::cerr << "JSON parse error: " <<  req.body << std::endl;
            return 200;
        }

        // make sure we have an actual event
        if(!envelope_obj.isObject() || !envelope_obj["type"].isString())
        {
            //we don't.
            std::cerr << "Invalid event JSON: " << req.body << std::endl;

            return 200;
        }

        //Check the token
        if(!args::get(verification_token).empty() && envelope_obj["token"].isString() &&
           (args::get(verification_token) != envelope_obj["token"].asString()))
        {
            // a token was specified, and it doesn't match what we got on the wire
            std::cerr << "Invalid token on event: " << req.body << std::endl;

            return 200;
        }

        auto envelope_type = envelope_obj["type"].asString();
        //This will be one of two things `url_verification` or `event_callback`

        if(envelope_type == "url_verification")
        {
            if(! envelope_obj["challenge"])
            {
                std::cerr << "Received Slack challenge request, but challenge string is invalid!" << std::endl;
                return 200;
            }
            std::cout << "Received Slack challenge \"" << envelope_obj["challenge"].asString() << "\"; responding!";
            return envelope_obj["challenge"].asString();
        }

        else if(envelope_type != "event_callback")
        {
            // a token was specified, and it doesn't match what we got on the wire
            std::cerr << "Unknown event envelope: " << req.body << std::endl;

            return 200;
        }

        // TODO optionally display the event envelope as well
//        http_event_envelope envelope{
//                envelope_obj["token"].asString(), //verification_token
//                token, //token
//                envelope_obj["api_app_id"].asString(), //api_app_id
//                {}
//        };
//        envelope.token.team_id = envelope_obj["team_id"].asString();

//        if(!envelope_obj["authed_users"].isNull() && envelope_obj["authed_users"].isArray())
//        {
//            for(const auto user_obj : envelope_obj["authed_users"])
//            {
//                envelope.authed_users.emplace_back(user_obj.asString());
//            }
//        }


        // Extract and examine the event from the event envelope
        Json::Value result_obj = envelope_obj["event"];

        // make sure we have an actual event
        if(!result_obj.isObject() || !result_obj["type"].isString())
        {
            //we don't.
            std::cerr << "Invalid event JSON: " << req.body << std::endl;

            return 200;
        }

        auto type = result_obj["type"].asString();
        if(result_obj["subtype"].isString())
        {
            type += "." + result_obj["subtype"].asString();
        }

        std::cout << "Received event \"" << type << "\"" << std::endl;
        std::cout << result_obj << std::endl;

        return 200;
    });

    server.await(); //run forever, basically, or until the server decides to kill itself.

    return 0;
}