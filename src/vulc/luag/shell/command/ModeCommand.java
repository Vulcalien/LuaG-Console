package vulc.luag.shell.command;

import vulc.luag.Console;
import vulc.luag.Console.Mode;
import vulc.luag.shell.Shell;

public class ModeCommand extends ShellCommand {

	public ModeCommand() {
		super("mode");
	}

	public void run(String[] args) {
		if(args.length < 1) {
			Shell.write("current mode:\n");
			if(Console.mode == Mode.DEVELOPER) {
				Shell.write("developer");
			} else {
				Shell.write("user");
			}
			Shell.write("\n\n");
			return;
		}

		String mode = args[0];
		if(mode.equals("d") || mode.equals("developer")) {
			Console.mode = Mode.DEVELOPER;
			Shell.write("switching to\n"
			            + "developer mode\n\n");
		} else if(mode.equals("u") || mode.equals("user")) {
			Console.mode = Mode.USER_SHELL;
			Shell.write("switching to\n"
			            + "user mode\n\n");
		} else {
			Console.die("Error:\n"
			            + "unrecognized mode\n"
			            + "try 'd' or 'u'");
		}
	}

	protected String getHelpMessage() {
		return "'mode <mode>'\n"
		       + "changes the console mode\n"
		       + "'d' is developer\n"
		       + "'u' is user";
	}

}
