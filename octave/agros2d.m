function [out]  = agros2d(command)
	agros2d_command = sprintf("agros2d-remote \"%s\"", command);
	[status, output] = system(agros2d_command);
	
	if (length(output) > 0)
		if ((length(output) > 45	) && (substr(output, 1, 46)) == 'Error: The connection was refused by the peer.')
			system('agros2d', 0, 'async');
			for i = 1:10
				sleep(1);
				if (system('ps | grep agros2d') ~= '')
					break;
			end
			
			output = agros2d(command);
  		end
	end
	
	out = output;
end