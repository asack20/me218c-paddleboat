clear
clc
hold off
shiftList = linspace(-381,381,100);

for k=1:length(shiftList)

    %CPList = [linspace(-381,-381,100)',linspace(-381,381,100)'];
    %CPList = [linspace(0,0,100)',linspace(-381,381,100)'];
    %CPList = [linspace(381,381,100)',linspace(-381,381,100)'];
    CPList = [linspace(shiftList(k),shiftList(k),100)',linspace(-381,381,100)'];

    for j=1:length(CPList(:,1))

        Size = 0;
        NumPts = 1;
        CenterPoint = CPList(j,:);

        InPointList = [[CenterPoint(1)*ones(NumPts,1);linspace(CenterPoint(1)-Size,CenterPoint(1)+Size,NumPts)'],[linspace(CenterPoint(2)-Size,CenterPoint(2)+Size,NumPts)';CenterPoint(2)*ones(NumPts,1)]];

        for i=1:length(InPointList(:,1))

            InPoint = InPointList(i,:);

            MidPoint = [(InPoint(1)+InPoint(2))/6, (InPoint(1)-InPoint(2))/6];

            OutPoint = [0,0];
            X = MidPoint(1);
            Y = MidPoint(2);
            if ((X>0) && (Y>0))
                OutPoint = [max(abs(X),abs(Y)), X-Y];
            elseif ((X>0) && (Y<0))
                OutPoint = [X+Y, max(abs(X),abs(Y))];
            elseif ((X<0) && (Y>0))
                OutPoint = [X+Y, -max(abs(X),abs(Y))];
            else
                OutPoint = [-max(abs(X),abs(Y)), X-Y];
            end
            OutPoint = OutPoint*3;

            plot(InPoint(1),InPoint(2),'ro','MarkerSize',2)
            hold on
            plot(OutPoint(1),OutPoint(2),'bo','MarkerSize',2)
            xlim([-381,381])
            ylim([-381,381])

        end
        %pause
        %hold off
    end
end