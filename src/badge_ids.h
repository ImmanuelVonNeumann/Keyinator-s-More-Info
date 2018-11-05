#pragma once
#include <map>
#include <string>

std::map<std::string, std::string> guids;
void init_guids() {
	guids.insert(std::pair<std::string, std::string>("1cb07348-34a4-4741-b50f-c41e584370f7", "Creator of TeamSpeak Addons"));
	guids.insert(std::pair<std::string, std::string>("50bbdbc8-0f2a-46eb-9808-602225b49627", "Registered during Gamescom 2016"));
	guids.insert(std::pair<std::string, std::string>("d95f9901-c42d-4bac-8849-7164fd9e2310", "Registered during Paris Games Week 2016"));
	guids.insert(std::pair<std::string, std::string>("62444179-0d99-42ba-a45c-c6b1557d079a", "Registered at Gamescom 2014"));
	guids.insert(std::pair<std::string, std::string>("d95f9901-c42d-4bac-8849-7164fd9e2310", "Registered at Paris Games Week 2014"));
	guids.insert(std::pair<std::string, std::string>("450f81c1-ab41-4211-a338-222fa94ed157", "Creator of at least 1 TeamSpeak Addon"));
	guids.insert(std::pair<std::string, std::string>("c9e97536-5a2d-4c8e-a135-af404587a472", "Creator of at least 3 TeamSpeak Addon"));
	guids.insert(std::pair<std::string, std::string>("94ec66de-5940-4e38-b002-970df0cf6c94", "Creator of at least 5 TeamSpeak Addon"));
	guids.insert(std::pair<std::string, std::string>("534c9582-ab02-4267-aec6-2d94361daa2a", "Visited TeamSpeak at Gamescom 2017"));
	guids.insert(std::pair<std::string, std::string>("34dbfa8f-bd27-494c-aa08-a312fc0bb240", "Gaming Hero at Gamescom 2017"));
	guids.insert(std::pair<std::string, std::string>("7d9fa2b1-b6fa-47ad-9838-c239a4ddd116", "MIFCOM | Entered Performance"));
	guids.insert(std::pair<std::string, std::string>("f81ad44d-e931-47d1-a3ef-5fd160217cf8", "4Netplayers customer"));
	guids.insert(std::pair<std::string, std::string>("f22c22f1-8e2d-4d99-8de9-f352dc26ac5b", "Rocket Beans TV Community"));
	guids.insert(std::pair<std::string, std::string>("64221fd1-706c-4bb2-ba55-996c39effa79", "MyTeamSpeak early adopter"));
	guids.insert(std::pair<std::string, std::string>("c3f823eb-5d5c-40f9-9dbd-3437d59a539d", "New MyTeamSpeak member"));
	guids.insert(std::pair<std::string, std::string>("935e5a2a-954a-44ca-aa7a-55c79285b601", "Discovered at E3 2018"));
	guids.insert(std::pair<std::string, std::string>("4eef1ecf-a0ea-423d-bfd0-496543a00305", "Visited TeamSpeak at Gamescom 2018"));
	guids.insert(std::pair<std::string, std::string>("24512806-f886-4440-b579-9e26e4219ef6", "Gamescom Exclusive Gaming Hero 2018"));
	guids.insert(std::pair<std::string, std::string>("b9c7d6ad-5b99-40fb-988c-1d02ab6cc130", "Found Tim Speak at Gamescom 2018"));
	guids.insert(std::pair<std::string, std::string>("6b187e83-873b-46b0-b2c2-a31af15e76a4", "TeamSpeak Merch Owner - 1st Edition"));
}

std::string guid_name(std::string guid) {
	std::string guid_name;
	guid_name = guids[guid];

	return guid_name;
}