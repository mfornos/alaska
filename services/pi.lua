-----------------------------------------------------
-- PI Service (dummy)
-----------------------------------------------------
-- Posts an approximation of PI to the broker
-----------------------------------------------------
-- Nilakantha
function pi(iters)
 i, ans, j = 0, 3, 2

 while i < iters do
  inc, i = 4.0 / (j * (j + 1) * (j + 2)), i + 1
  ans, j = ans + (i % 2 == 1 and inc or -inc), j + 2
 end
 return ans
end

local res = pi(28)
-- Sends the result to 'aw.res.pi' mbox w/ 'pi.28' subject
sendTo('pi.28', res, 'aw.res.pi')
-- Sends the result to 'numbers' stream w/ 'pi.28' subject
send('pi.28', res, 'numbers')
