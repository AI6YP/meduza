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
	length = fix (rows (data_in) / 3);
	data_in = resize (data_in, 3 * length, 1);
	size (data_in)
	data_in = reshape (data_in, 3, (length));
	size (data_in)
	range = (0:1:length-1) / 800;
	plot (range, data_in (1,:), range, data_in (2,:), range, data_in (3,:));
end %function

%!demo
%! f1 ('../logs/20130212_01.bin.f1', 300000, "int16");
