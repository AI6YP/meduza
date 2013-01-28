% $Id$
% file_in - input stream file name

function f1 (file_in, num_of_elements, element_type)
	data_in = [];
	count_ = 0;

	fid_in = fopen (file_in, 'r');
	if (fid_in < 0)
		s1 = sprintf ('Uable to open file %s for reading\n', file_in);
		disp (s1);
	else
		[data_in, count_] = fread (fid_in, num_of_elements, element_type);
	end
	fclose (fid_in);

	plot (data_in);
end %function

%!demo
%! f1 ('../logs/20130127_01.bin.f1', 2000, "int16");
