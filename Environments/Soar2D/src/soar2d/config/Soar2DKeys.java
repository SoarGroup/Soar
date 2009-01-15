package soar2d.config;

public class Soar2DKeys {
	public static String clientKey(String id) {
		return Soar2DKeys.clients.clients_ + id;
	}
	public class clients {
		public static final String active_clients = "clients.active_clients";
		
		private static final String clients_ = "clients.";
		public static final String command = "command";
		public static final String timeout = "timeout";
		public static final String after = "after";
	}
	public static String clients_command(String name) {
		return clients.clients_ + name + clients.command;
	}
	public static String clients_timeout(String name) {
		return clients.clients_ + name + clients.timeout;
	}
	public static String clients_after(String name) {
		return clients.clients_ + name + clients.after;
	}

	public class eaters {
		public static final String vision = "eaters.vision";
		public static final String wall_penalty = "eaters.wall_penalty";
		public static final String jump_penalty = "eaters.jump_penalty";
		public static final String low_probability = "eaters.low_probability";
		public static final String high_probability = "eaters.high_probability";
	}
	
	public class general {
		public static final String async = "general.async";
		public static final String default_points = "general.default_points";
		public static final String force_human = "general.force_human";
		public static final String game = "general.game";
		public static final String hidemap = "general.hidemap";
		public static final String map = "general.map";
		public static final String nogui = "general.nogui";
		public static final String seed = "general.seed";
		public static final String tosca = "general.tosca";
		
		public class logging {
			public static final String console = "general.logging.console";
			public static final String file = "general.logging.file";
			public static final String level = "general.logging.level";
			public static final String time = "general.logging.time";
			public static final String soar_print = "general.logging.soar_print";
		}
		
		public class soar {
			public static final String max_memory_usage = "general.soar.max_memory_usage";
			public static final String port = "general.soar.port";
			public static final String remote = "general.soar.remote";
			public static final String spawn_debuggers = "general.soar.spawn_debuggers";
			public static final String watch_0 = "general.soar.watch_0";
			public static final String metadata = "general.soar.metadata";
		}
	}

	public static String playerKey(String id) {
		return Soar2DKeys.players.players_ + id;
	}
	public class players {
		public static final String active_players = "players.active_players";
		
		private static final String players_ = "players.";
		public static final String name = "name";
		public static final String productions = "productions";
		public static final String color = "color";
		public static final String pos = "pos";
		public static final String facing = "facing";
		public static final String points = "points";
		public static final String energy = "energy";
		public static final String health = "health";
		public static final String missiles = "missiles";
		public static final String shutdown_commands = "shutdown_commands";
	}
	
	public class room {
		public static final String colored_rooms = "room.colored_rooms";
		public static final String speed = "room.speed";
		public static final String cell_size = "room.cell_size";
		public static final String cycle_time_slice = "room.cycle_time_slice";
		public static final String vision_cone = "room.vision_cone";
		public static final String rotate_speed = "room.rotate_speed";
		public static final String blocks_block = "room.blocks_block";
		public static final String continuous = "room.continuous";
		public static final String zero_is_east = "room.zero_is_east";
	}
	
	public class tanksoar {
		public static final String default_missiles = "tanksoar.default_missiles";
		public static final String default_energy = "tanksoar.default_energy";
		public static final String default_health = "tanksoar.default_health";
		public static final String collision_penalty = "tanksoar.collision_penalty";
		public static final String max_missile_packs = "tanksoar.max_missile_packs";
		public static final String missile_pack_respawn_chance = "tanksoar.missile_pack_respawn_chance";
		public static final String shield_energy_usage = "tanksoar.shield_energy_usage";
		public static final String missile_hit_award = "tanksoar.missile_hit_award";
		public static final String missile_hit_penalty = "tanksoar.missile_hit_penalty";
		public static final String kill_award = "tanksoar.kill_award";
		public static final String kill_penalty = "tanksoar.kill_penalty";
		public static final String radar_width = "tanksoar.radar_width";
		public static final String radar_height = "tanksoar.radar_height";
		public static final String max_smell_distance = "tanksoar.max_smell_distance";
		public static final String missile_reset_threshold = "tanksoar.missile_reset_threshold";
	}
	
	public class taxi {
		public static final String disable_fuel = "taxi.disable_fuel";
		public static final String fuel_starting_minimum = "taxi.fuel_starting_minimum";
		public static final String fuel_starting_maximum = "taxi.fuel_starting_maximum";
		public static final String fuel_maximum = "taxi.fuel_maximum";
	}
	
	public class terminals {
		public static final String max_updates = "terminals.max_updates";
		public static final String agent_command = "terminals.agent_command";
		public static final String points_remaining = "terminals.points_remaining";
		public static final String winning_score = "terminals.winning_score";
		public static final String food_remaining = "terminals.food_remaining";
		public static final String unopened_boxes = "terminals.unopened_boxes";
		public static final String max_runs = "terminals.max_runs";
		public static final String fuel_remaining = "terminals.fuel_remaining";
		public static final String passenger_delivered = "terminals.passenger_delivered";
		public static final String passenger_pick_up = "terminals.passenger_pick_up";
	}
	
	public class preferences {
		public static final String last_productions = "last_productions";
		public static final String window_position = "window_position";
	}
}
