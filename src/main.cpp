#include <atomic>
#include <iostream>
#include <string>

#include <dpp/dpp.h>

using namespace std::string_literals;

// Zero iff all get_*_async calls' callbacks have completed.
std::atomic_int init_counter = 0;

void get_guild_async(dpp::guild &guild, dpp::snowflake id, dpp::cluster &bot)
{
	init_counter++;
	bot.guild_get(id,
				  [&](dpp::confirmation_callback_t const &callback)
				  {
					  if (callback.is_error())
					  {
						  std::cerr << "Failed to get guild.\n";
						  std::exit(1);
					  }
					  guild = callback.get<dpp::guild>();
					  init_counter--;
				  });
}
void get_channel_async(dpp::channel &channel, dpp::snowflake id, dpp::cluster &bot)
{
	init_counter++;
	bot.channel_get(id,
					[&](dpp::confirmation_callback_t const &callback)
					{
						if (callback.is_error())
						{
							std::cerr << "Failed to get channel.\n";
							std::exit(1);
						}
						channel = callback.get<dpp::channel>();
						init_counter--;
					});
}
void get_user_async(dpp::user &user, dpp::snowflake id, dpp::cluster &bot)
{
	init_counter++;
	bot.user_get(id,
				 [&](dpp::confirmation_callback_t const &callback)
				 {
					 if (callback.is_error())
					 {
						 std::cerr << "Failed to get user.\n";
						 std::exit(1);
					 }
					 user = callback.get<dpp::user_identified>();
					 init_counter--;
				 });
}

int main(int argc, char const *argv[])
{
	if (argc != 7)
	{
		std::cerr << "Expected 6 arguments.\n";
		return 1;
	}

	std::string const TOKEN = argv[1], GUILD = argv[2], ADMIN = argv[3], HONEYPOT_CHANNEL = argv[4], ADMIN_CHANNEL = argv[5], ADMIN_NAME = argv[6];

	if (HONEYPOT_CHANNEL == ADMIN_CHANNEL)
	{
		std::cerr << "Honeypot and Admin channels must not be the same.\n";
		return 1;
	}

	dpp::cluster bot(TOKEN);

	dpp::guild	 guild;
	dpp::channel honeypot_channel, admin_channel;
	dpp::user	 admin;

	bot.on_ready(
		[&](dpp::ready_t const &event)
		{
			if (dpp::run_once<struct on_ready_once>())
			{
				get_guild_async(guild, GUILD, bot);
				get_channel_async(honeypot_channel, HONEYPOT_CHANNEL, bot);
				get_channel_async(admin_channel, ADMIN_CHANNEL, bot);
				get_user_async(admin, ADMIN, bot);
			}
		});

	auto const message_create_callback = [&](dpp::message_create_t const &event)
	{
		auto const &msg = event.msg;
		if (init_counter || msg.channel_id != honeypot_channel.id || msg.author == bot.me || msg.author == admin)
			return;

		std::cout << "Banning " << msg.author.username << '\n' << std::flush;

		dpp::message admin_notif(admin_channel.id,
								 admin.get_mention() + ": banning "s + msg.author.get_mention() + " for suspected spam, please review."s);
		admin_notif.set_allowed_mentions(false, false, false, false, {admin.id}, {});
		bot.message_create(admin_notif);

		dpp::message author_notif(
			"You have been banned from the "s + guild.name +
			" server for suspected spam. If you believe this was an error, or your account was hacked and you have since recovered it, please email "s +
			ADMIN_NAME + "."s);
		auto const dm_create_callback = [&](dpp::confirmation_callback_t const &callback)
		{
			if (callback.is_error())
				std::cout << "Failed to send DM notification.\n" << std::flush;
		};
		bot.direct_message_create(msg.author.id, author_notif, dm_create_callback);

		bot.set_audit_reason("Suspected spam");
		bot.guild_ban_add(guild.id, msg.author.id, 3600);
	};
	bot.on_message_create(message_create_callback);

	bot.start(dpp::st_wait);

	return 0;
}
